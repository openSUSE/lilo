/* $Id$ */
/*
 * arch/ppc/boot/simple/misc-prep.c
 *
 * Maintainer: Tom Rini <trini@kernel.crashing.org>
 *
 * In the past: Gary Thomas, Cort Dougan <cort@cs.nmt.edu>
 */

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <residual.h>
#include <string.h>
#include <mpc10x.h>
#include <prom.h>
#include <page.h>
#include <pci_ids.h>
#include <serial.h>
#include <bootinfo.h>
#include <cmdline.h>
#include <byteswab.h>

extern char _start[];
extern char _end[];
extern char _vmlinuz_start[];
extern char _vmlinuz_end[];
extern char _initrd_start[];
extern char _initrd_end[];

extern void udelay(long delay);

static int no_keyb_present;	/* keyboard controller is present by default */

static RESIDUAL hold_resid_buf;
static RESIDUAL *hold_residual = &hold_resid_buf;

char *vidmem = (char *)0xC00B8000;
int lines = 25, cols = 80;
static int orig_x, orig_y = 24;

extern int CRT_tstc(void);
extern int CRT_getc(void);
extern int vga_init(unsigned char *ISA_mem);
extern unsigned long serial_init(int chan, void *ignored);
extern void serial_fixups(void);
extern void disable_6xx_mmu(void);

static unsigned long com_port;

/* Very simple inb/outb routines.  We declare ISA_io to be 0 above, and
 * then modify it on platforms which need to.  We do it like this
 * because on some platforms we give inb/outb an exact location, and
 * on others it's an offset from a given location. -- Tom
 */
static unsigned char *ISA_io;

static void ISA_init(unsigned char *base)
{
	ISA_io = base;
}

void outb(int port, unsigned char val)
{
	/* Ensure I/O operations complete */
	__asm__ volatile ("eieio");
	ISA_io[port] = val;
}

unsigned char inb(int port)
{
	/* Ensure I/O operations complete */
	__asm__ volatile ("eieio");
	return (ISA_io[port]);
}

static inline int in_8(volatile unsigned char *addr)
{
	int ret;

	__asm__ __volatile__("lbz%U1%X1 %0,%1;\n"
			     "twi 0,%0,0;\n" "isync":"=r"(ret):"m"(*addr));
	return ret;
}
static inline unsigned in_le32(volatile unsigned *addr)
{
	unsigned ret;

	__asm__ __volatile__("lwbrx %0,0,%1;\n"
			     "twi 0,%0,0;\n"
			     "isync":"=r"(ret): "r"(addr), "m"(*addr));
	return ret;
}

static inline void out_be32(volatile unsigned *addr, int val)
{
	__asm__ __volatile__("stw%U0%X0 %1,%0; eieio":"=m"(*addr):"r"(val));
}

static void writel(unsigned int val, unsigned int address)
{
	/* Ensure I/O operations complete */
	__asm__ volatile ("eieio");
	*(unsigned int *)address = cpu_to_le32(val);
}

#define PCI_DEVFN(slot,func)	((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_CFG_ADDR(dev,off)	((0x80<<24) | (dev<<8) | (off&0xfc))
#define PCI_CFG_DATA(off)	(MPC10X_MAPA_CNFG_DATA+(off&3))

static void
pci_read_config_32(unsigned char devfn, unsigned char offset, unsigned int *val)
{
	/* Ensure I/O operations complete */
	__asm__ volatile ("eieio");
	*(unsigned int *)PCI_CFG_ADDR(devfn, offset) =
	    cpu_to_le32(MPC10X_MAPA_CNFG_ADDR);
	/* Ensure I/O operations complete */
	__asm__ volatile ("eieio");
	*val = le32_to_cpu(*(unsigned int *)PCI_CFG_DATA(offset));
	return;
}

/*
 * cursor() sets an offset (0-1999) into the 80x25 text area.
 */
static void cursor(int x, int y)
{
	int pos = (y * cols) + x;
	outb(0x3D4, 14);
	outb(0x3D5, pos >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, pos);
}

static void scroll(void)
{
	int i;

	memcpy(vidmem, vidmem + cols * 2, (lines - 1) * cols * 2);
	for (i = (lines - 1) * cols * 2; i < lines * cols * 2; i += 2)
		vidmem[i] = ' ';
}

static int puts(const char *s, int len)
{
	int x, y;
	int count = 0;
	char c;

	x = orig_x;
	y = orig_y;

	while ((c = *s++) != '\0') {
		if (count == len)
			break;
		serial_putc(com_port, c);
		if (c == '\n') {
			serial_putc(com_port, '\r');
			x = 0;
			if (++y >= lines) {
				scroll();
				y--;
			}
		} else if (c == '\b') {
			if (x > 0) {
				x--;
			}
		} else {
			vidmem[(x + cols * y) * 2] = c;
			if (++x >= cols) {
				x = 0;
				if (++y >= lines) {
					scroll();
					y--;
				}
			}
		}
		count++;
	}

	cursor(x, y);

	orig_x = x;
	orig_y = y;

	return count;
}

void mdelay(int ms)
{
	if (promptr) {
		ms = of1275_milliseconds() + ms;
		while(of1275_milliseconds() < ms);
	} else {
		while (ms-- > 0)
			udelay(1000);
	}
}

int read(void *buf, int buflen)
{
	unsigned char *p = buf;
	int ret = 0;
	if (promptr && no_keyb_present)
		return of1275_read(stdin, buf, buflen);

	if (serial_tstc(com_port)) {
		*p = serial_getc(com_port);
		ret = 1;
	} else {
		if (!no_keyb_present)
			if (CRT_tstc()) {
				*p = CRT_getc();
				ret = 1;
			}
	}
	return ret;
}

int write(void *buf, int buflen)
{
	if (promptr)
		return of1275_write(stdout, buf, buflen);
	return puts(buf, buflen);
}

/*
 * *** WARNING - A BAT MUST be set to access the PCI config addr/data regs ***
 */

/*
 * PCI config space macros, similar to indirect_xxx and early_xxx macros.
 * We assume bus 0.
 */
#define MPC10X_CFG_read(val, addr, type, op)	*val = op((type)(addr))
#define MPC10X_CFG_write(val, addr, type, op)	op((type *)(addr), (val))

#define MPC10X_PCI_OP(rw, size, type, op, mask)			 	\
static void								\
mpc10x_##rw##_config_##size(unsigned int *cfg_addr, 			\
		unsigned int *cfg_data, int devfn, int offset,		\
		type val)						\
{									\
	out_be32(cfg_addr, 						\
		 ((offset & 0xfc) << 24) | (devfn << 16)		\
		 | (0 << 8) | 0x80);					\
	MPC10X_CFG_##rw(val, cfg_data + (offset & mask), type, op);	\
	return;    					 		\
}

MPC10X_PCI_OP(read, byte, unsigned char *, in_8, 3)
    MPC10X_PCI_OP(read, dword, unsigned int *, in_le32, 0)

/*
 * Read the memory controller registers to determine the amount of memory in
 * the system.  This assumes that the firmware has correctly set up the memory
 * controller registers.  On CONFIG_PPC_PREP, we know we are being called
 * under a PReP memory map. On all other machines, we assume we are under
 * a CHRP memory map.  Further, on CONFIG_PPC_MULTIPLATFORM we must rename
 * this function.
 */
static unsigned long mpc10x_get_mem_size(void)
{
	unsigned int *config_addr, *config_data, val;
	unsigned long start, end, total, offset;
	int i;
	unsigned char bank_enables;

	config_addr = (unsigned int *)MPC10X_MAPA_CNFG_ADDR;
	config_data = (unsigned int *)MPC10X_MAPA_CNFG_DATA;

	mpc10x_read_config_byte(config_addr, config_data, PCI_DEVFN(0, 0),
				MPC10X_MCTLR_MEM_BANK_ENABLES, &bank_enables);

	total = 0;

	for (i = 0; i < 8; i++) {
		if (bank_enables & (1 << i)) {
			offset = MPC10X_MCTLR_MEM_START_1 + ((i > 3) ? 4 : 0);
			mpc10x_read_config_dword(config_addr, config_data,
						 PCI_DEVFN(0, 0), offset, &val);
			start = (val >> ((i & 3) << 3)) & 0xff;

			offset =
			    MPC10X_MCTLR_EXT_MEM_START_1 + ((i > 3) ? 4 : 0);
			mpc10x_read_config_dword(config_addr, config_data,
						 PCI_DEVFN(0, 0), offset, &val);
			val = (val >> ((i & 3) << 3)) & 0x03;
			start = (val << 28) | (start << 20);

			offset = MPC10X_MCTLR_MEM_END_1 + ((i > 3) ? 4 : 0);
			mpc10x_read_config_dword(config_addr, config_data,
						 PCI_DEVFN(0, 0), offset, &val);
			end = (val >> ((i & 3) << 3)) & 0xff;

			offset = MPC10X_MCTLR_EXT_MEM_END_1 + ((i > 3) ? 4 : 0);
			mpc10x_read_config_dword(config_addr, config_data,
						 PCI_DEVFN(0, 0), offset, &val);
			val = (val >> ((i & 3) << 3)) & 0x03;
			end = (val << 28) | (end << 20) | 0xfffff;

			total += (end - start + 1);
		}
	}

	return total;
}
static unsigned long get_mem_size(void)
{
	unsigned int pci_viddid, pci_did;

	/* First, figure out what kind of host bridge we are on.  If it's
	 * an MPC10x, we can ask it directly how much memory it has.
	 * Otherwise, see if the residual data has anything.  This isn't
	 * the best way, but it can be the only way.  If there's nothing,
	 * assume 32MB. -- Tom.
	 */
	/* See what our host bridge is. */
	pci_read_config_32(0x00, 0x00, &pci_viddid);
	pci_did = (pci_viddid & 0xffff0000) >> 16;
	/* See if we are on an MPC10x. */
	if (((pci_viddid & 0xffff) == PCI_VENDOR_ID_MOTOROLA)
	    && ((pci_did == PCI_DEVICE_ID_MOTOROLA_MPC105)
		|| (pci_did == PCI_DEVICE_ID_MOTOROLA_MPC106)
		|| (pci_did == PCI_DEVICE_ID_MOTOROLA_MPC107)))
		return mpc10x_get_mem_size();
	/* If it's not, see if we have anything in the residual data. */
	else if (hold_residual && hold_residual->TotalMemory)
		return hold_residual->TotalMemory;
	else if (promptr) {
		/*
		 * This is a 'best guess' check.  We want to make sure
		 * we don't try this on a PReP box without OF
		 *     -- Cort
		 */
		while (promptr) {
			phandle dev_handle;
			int mem_info[2];

			/* get handle to memory description */
			if (!(dev_handle = of1275_finddevice("/memory@0")))
				break;

			/* get the info */
			if (of1275_getprop(dev_handle, "reg", mem_info,
					   sizeof(mem_info)) != 8)
				break;

			return mem_info[1];
		}
	}

	/* Fall back to hard-coding 32MB. */
	return 32 * 1024 * 1024;
}

static struct bi_record *birec;

static struct bi_record *__bootinfo_build(struct bi_record *rec,
					  unsigned long tag, unsigned long size,
					  void *data)
{
	/* set the tag */
	rec->tag = tag;

	/* if the caller has any data, copy it */
	if (size)
		memcpy(rec->data, (char *)data, size);

	/* set the record size */
	rec->size = sizeof(struct bi_record) + size;

	/* advance to the next available space */
	rec = (struct bi_record *)((unsigned long)rec + rec->size);

	return rec;
}

static void bootinfo_init(struct bi_record *rec)
{

	/* save start of birec area */
	birec = rec;

	/* create an empty list */
	rec = __bootinfo_build(rec, BI_FIRST, 0, NULL);
	(void)__bootinfo_build(rec, BI_LAST, 0, NULL);

}

static void bootinfo_append(unsigned long tag, unsigned long size, void *data)
{

	struct bi_record *rec = birec;

	/* paranoia */
	if ((rec == NULL) || (rec->tag != BI_FIRST))
		return;

	/* find the last entry in the list */
	while (rec->tag != BI_LAST)
		rec = (struct bi_record *)((unsigned long)rec + rec->size);

	/* overlay BI_LAST record with new one and tag on a new BI_LAST */
	rec = __bootinfo_build(rec, tag, size, data);
	(void)__bootinfo_build(rec, BI_LAST, 0, NULL);
}

static
struct bi_record *decompress_kernel(unsigned long load_addr, int num_words,
				    unsigned long cksum)
{
	struct bi_record *rec;
	unsigned long initrd_loc, TotalMemory;
	unsigned long zimage_start, zimage_size, initrd_size;

	com_port = serial_init(0, NULL);

	/*
	 * Call get_mem_size(), which is memory controller dependent,
	 * and we must have the correct file linked in here.
	 */
	TotalMemory = get_mem_size();

	if (promptr)
		printf("Open Firmware: 0x%p\n", promptr);

	/*
	 * Reveal where we were loaded at and where we
	 * were relocated to.
	 */
	printf("loaded at:     0x%08lx 0x%08lx\n", load_addr,
	       (load_addr + (4 * num_words)));
	if (load_addr != (unsigned long)_start)
		printf("relocated to:  0x%08lx 0x%08p\n", _start,
		       (_start + (4 * num_words)));

	/*
	 * We link ourself to 0x00800000.  When we run, we relocate
	 * ourselves there.  So we just need _vmlinuz_start for the
	 * start. -- Tom
	 */
	zimage_start = (unsigned long)_vmlinuz_start;
	zimage_size = (unsigned long)(_vmlinuz_end - _vmlinuz_start);

	initrd_size = (unsigned long)(_initrd_end - _initrd_start);

	/*
	 * The zImage and initrd will be between start and _end, so they've
	 * already been moved once.  We're good to go now. -- Tom
	 */
	printf("zimage at:     0x%08p 0x%08p\n",
	       zimage_start, zimage_size + zimage_start);

	if (initrd_size)
		printf("initrd at:     0x%08p 0x%08p\n",
		       _initrd_start, _initrd_end);

	if (!no_keyb_present)
		CRT_tstc();	/* Forces keyboard to be initialized */

	get_cmdline(_builtin_cmd_line.string, strlen(_builtin_cmd_line.string), sizeof(_builtin_cmd_line.string));

	zimage_size =
	    gunzip(0x0, 0x400000, zimage_start, zimage_size, "kernel");

	/* get the bi_rec address */
	rec = bootinfo_addr(zimage_size);

	/* We need to make sure that the initrd and bi_recs do not
	 * overlap. */
	if (initrd_size) {
		unsigned long rec_loc = (unsigned long)rec;
		initrd_loc = (unsigned long)(&_initrd_start);
		/* If the bi_recs are in the middle of the current
		 * initrd, move the initrd to the next MB
		 * boundary. */
		if ((rec_loc > initrd_loc) &&
		    ((initrd_loc + initrd_size) > rec_loc)) {
			initrd_loc = _ALIGN((unsigned long)(zimage_size)
					    + (2 << 20) - 1, (2 << 20));
			memmove((void *)initrd_loc, &_initrd_start,
				initrd_size);
			printf("initrd moved: 0x%08lx 0x%08lx\n", initrd_loc,
			       initrd_loc + initrd_size);
		}
	}

	printf("birecs @ 0x%08p\n", rec);
	bootinfo_init(rec);
	if (TotalMemory)
		bootinfo_append(BI_MEMSIZE, sizeof(int), (void *)&TotalMemory);

	bootinfo_append(BI_CMD_LINE, strlen(_builtin_cmd_line.string) + 1, (void *)_builtin_cmd_line.string);

	/* add a bi_rec for the initrd if it exists */
	if (initrd_size) {
		unsigned long initrd[2];

		initrd[0] = initrd_loc;
		initrd[1] = initrd_size;

		bootinfo_append(BI_INITRD, sizeof(initrd), &initrd);
	}
	printf("Now booting the kernel\n");

	return rec;
}

unsigned long
load_kernel(unsigned long load_addr, int num_words, unsigned long cksum,
	    RESIDUAL * residual, void *OFW)
{
	int start_multi = 0;
	unsigned int pci_viddid, pci_did, tulip_pci_base, tulip_base;

	/* If we have Open Firmware, initialise it immediately */
	if (OFW) {
		char tmp[128];

		of1275_prominit(OFW);

		of1275_instance_to_path(stdin, tmp, sizeof(tmp));
		printf("stdin  '%s'\n", tmp);
		of1275_getprop(of1275_instance_to_package(stdin), "device_type", tmp, sizeof(tmp));
		printf("type   '%s'\n", tmp);
		if (strcmp("serial", tmp) == 0)
			no_keyb_present = 1;
		of1275_instance_to_path(stdout, tmp, sizeof(tmp));
		printf("stdout '%s'\n", tmp);
	}

	ISA_init((unsigned char *)0x80000000);

	vga_init((unsigned char *)0xC0000000);

	if (residual) {
		/* Is this Motorola PPCBug? */
		if ((1 & residual->VitalProductData.FirmwareSupports) &&
		    (1 == residual->VitalProductData.FirmwareSupplier)) {
			unsigned char base_mod;
			unsigned char board_type = inb(0x801) & 0xF0;

			/*
			 * Reset the onboard 21x4x Ethernet
			 * Motorola Ethernet is at IDSEL 14 (devfn 0x70)
			 */
			pci_read_config_32(0x70, 0x00, &pci_viddid);
			pci_did = (pci_viddid & 0xffff0000) >> 16;
			/* Be sure we've really found a 21x4x chip */
			if (((pci_viddid & 0xffff) == PCI_VENDOR_ID_DEC) &&
			    ((pci_did == PCI_DEVICE_ID_DEC_TULIP_FAST) ||
			     (pci_did == PCI_DEVICE_ID_DEC_TULIP) ||
			     (pci_did == PCI_DEVICE_ID_DEC_TULIP_PLUS) ||
			     (pci_did == PCI_DEVICE_ID_DEC_21142))) {
				pci_read_config_32(0x70, 0x10, &tulip_pci_base);
				/* Get the physical base address */
				tulip_base =
				    (tulip_pci_base & ~0x03UL) + 0x80000000;
				/* Strobe the 21x4x reset bit in CSR0 */
				writel(0x1, tulip_base);
			}

			/* If this is genesis 2 board then check for no
			 * keyboard controller and more than one processor.
			 */
			if (board_type == 0xe0) {
				base_mod = inb(0x803);
				/* if a MVME2300/2400 or a Sitka then no keyboard */
				if ((base_mod == 0xFA) || (base_mod == 0xF9) ||
				    (base_mod == 0xE1)) {
					no_keyb_present = 1;	/* no keyboard */
				}
			}
			/* If this is a multiprocessor system then
			 * park the other processor so that the
			 * kernel knows where to find them.
			 */
			if (residual->MaxNumCpus > 1)
				start_multi = 1;
		}
		memcpy(hold_residual, residual, sizeof(RESIDUAL));
	}

	/* Call decompress_kernel */
	decompress_kernel(load_addr, num_words, cksum);

	if (start_multi) {
		residual->VitalProductData.SmpIar = (unsigned long)0xc0;
		residual->Cpus[1].CpuState = CPU_GOOD;
		hold_residual->VitalProductData.Reserved5 = 0xdeadbeef;
	}

	/* Now go and clear out the BATs and ensure that our MSR is
	 * correct .*/
	disable_6xx_mmu();

	/* Make r3 be a pointer to the residual data. */
	return (unsigned long)hold_residual;
}
