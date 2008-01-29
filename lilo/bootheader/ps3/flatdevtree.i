# 1 "flatdevtree.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "flatdevtree.c"
# 25 "flatdevtree.c"
# 1 "../include/string.h" 1




# 1 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stddef.h" 1 3 4
# 152 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stddef.h" 3 4
typedef int ptrdiff_t;
# 214 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stddef.h" 3 4
typedef unsigned int size_t;
# 326 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stddef.h" 3 4
typedef long int wchar_t;
# 6 "../include/string.h" 2

extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern char *strcat(char *dest, const char *src);
extern int strcmp(const char *s1, const char *s2);
extern size_t strlen(const char *s);
extern size_t strnlen(const char *s, size_t count);

extern unsigned long simple_strtoul(const char *cp, char **endp,
        unsigned int base);
extern long simple_strtol(const char *cp, char **endp, unsigned int base);

extern void *memset(void *s, int c, size_t n);
extern void *memmove(void *dest, const void *src, unsigned long n);
extern void *memcpy(void *dest, const void *src, unsigned long n);
extern int memcmp(const void *s1, const void *s2, size_t n);
# 26 "flatdevtree.c" 2
# 1 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stddef.h" 1 3 4
# 27 "flatdevtree.c" 2
# 1 "flatdevtree.h" 1
# 20 "flatdevtree.h"
# 1 "../include/flatdevtree_env.h" 1
# 13 "../include/flatdevtree_env.h"
# 1 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stdarg.h" 1 3 4
# 43 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 105 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stdarg.h" 3 4
typedef __gnuc_va_list va_list;
# 14 "../include/flatdevtree_env.h" 2
# 1 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stddef.h" 1 3 4
# 15 "../include/flatdevtree_env.h" 2
# 1 "../include/types.h" 1
# 16 "../include/types.h"
typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;
# 16 "../include/flatdevtree_env.h" 2
# 1 "../include/string.h" 1
# 17 "../include/flatdevtree_env.h" 2
# 1 "../include/stdio.h" 1







extern int printf(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
extern int sprintf(char *buf, const char *fmt, ...) __attribute__ ((format(printf, 2, 3)));
extern int vsprintf(char *buf, const char *fmt, va_list args) __attribute__ ((format(printf, 2, 0)));






extern int putc(int c);
extern int putchar(int c);
extern int getchar(int block);

extern int fputs(char *str, void *f);

extern int write(void *buf, int buflen);
extern int read(void *buf, int buflen);
# 18 "../include/flatdevtree_env.h" 2
# 1 "../include/ops.h" 1
# 14 "../include/ops.h"
# 1 "/usr/lib/gcc/powerpc64-suse-linux/4.1.2/include/stddef.h" 1 3 4
# 15 "../include/ops.h" 2







typedef void (*kernel_entry_t)(unsigned long r3, unsigned long r4, void *r5);


struct platform_ops {
 void (*fixups)(void);
 void (*image_hdr)(const void *);
 void * (*malloc)(unsigned long size);
 void (*free)(void *ptr);
 void * (*realloc)(void *ptr, unsigned long size);
 void (*exit)(void);
 void * (*vmlinux_alloc)(unsigned long size);
};
extern struct platform_ops platform_ops;


struct dt_ops {
 void * (*finddevice)(const char *name);
 int (*getprop)(const void *phandle, const char *name, void *buf,
   const int buflen);
 int (*setprop)(const void *phandle, const char *name,
   const void *buf, const int buflen);
 void *(*get_parent)(const void *phandle);

 void *(*create_node)(const void *parent, const char *name);
 void *(*find_node_by_prop_value)(const void *prev,
                                  const char *propname,
                                  const char *propval, int proplen);
 unsigned long (*finalize)(void);
 char *(*get_path)(const void *phandle, char *buf, int len);
};
extern struct dt_ops dt_ops;


struct console_ops {
 int (*open)(void);
 void (*write)(const char *buf, int len);
 void (*edit_cmdline)(char *buf, int len);
 void (*close)(void);
 void *data;
};
extern struct console_ops console_ops;


struct serial_console_data {
 int (*open)(void);
 void (*putc)(unsigned char c);
 unsigned char (*getc)(void);
 u8 (*tstc)(void);
 void (*close)(void);
};

struct loader_info {
 void *promptr;
 unsigned long initrd_addr, initrd_size;
 char *cmdline;
 int cmdline_len;
};
extern struct loader_info loader_info;

void start(void);
int ft_init(void *dt_blob, unsigned int max_size, unsigned int max_find_device);
int serial_console_init(void);
int ns16550_console_init(void *devp, struct serial_console_data *scdp);
int mpsc_console_init(void *devp, struct serial_console_data *scdp);
int cpm_console_init(void *devp, struct serial_console_data *scdp);
int mpc5200_psc_console_init(void *devp, struct serial_console_data *scdp);
int uartlite_console_init(void *devp, struct serial_console_data *scdp);
void *simple_alloc_init(char *base, unsigned long heap_size,
   unsigned long granularity, unsigned long max_allocs);
extern void flush_cache(void *, unsigned long);
int dt_xlate_reg(void *node, int res, unsigned long *addr, unsigned long *size);
int dt_xlate_addr(void *node, u32 *buf, int buflen, unsigned long *xlated_addr);
int dt_is_compatible(void *node, const char *compat);
void dt_get_reg_format(void *node, u32 *naddr, u32 *nsize);

static inline void *finddevice(const char *name)
{
 return (dt_ops.finddevice) ? dt_ops.finddevice(name) : ((void *)0);
}

static inline int getprop(void *devp, const char *name, void *buf, int buflen)
{
 return (dt_ops.getprop) ? dt_ops.getprop(devp, name, buf, buflen) : -1;
}

static inline int setprop(void *devp, const char *name,
                          const void *buf, int buflen)
{
 return (dt_ops.setprop) ? dt_ops.setprop(devp, name, buf, buflen) : -1;
}






static inline int setprop_str(void *devp, const char *name, const char *buf)
{
 if (dt_ops.setprop)
  return dt_ops.setprop(devp, name, buf, strlen(buf) + 1);

 return -1;
}

static inline void *get_parent(const char *devp)
{
 return dt_ops.get_parent ? dt_ops.get_parent(devp) : ((void *)0);
}

static inline void *create_node(const void *parent, const char *name)
{
 return dt_ops.create_node ? dt_ops.create_node(parent, name) : ((void *)0);
}


static inline void *find_node_by_prop_value(const void *prev,
                                            const char *propname,
                                            const char *propval, int proplen)
{
 if (dt_ops.find_node_by_prop_value)
  return dt_ops.find_node_by_prop_value(prev, propname,
                                        propval, proplen);

 return ((void *)0);
}

static inline void *find_node_by_prop_value_str(const void *prev,
                                                const char *propname,
                                                const char *propval)
{
 return find_node_by_prop_value(prev, propname, propval,
                                strlen(propval) + 1);
}

static inline void *find_node_by_devtype(const void *prev,
                                         const char *type)
{
 return find_node_by_prop_value_str(prev, "device_type", type);
}

void dt_fixup_memory(u64 start, u64 size);
void dt_fixup_cpu_clocks(u32 cpufreq, u32 tbfreq, u32 busfreq);
void dt_fixup_clock(const char *path, u32 freq);
void dt_fixup_mac_address(u32 index, const u8 *addr);
void __dt_fixup_mac_addresses(u32 startindex, ...);




static inline void *find_node_by_linuxphandle(const u32 linuxphandle)
{
 return find_node_by_prop_value(((void *)0), "linux,phandle",
   (char *)&linuxphandle, sizeof(u32));
}

static inline char *get_path(const void *phandle, char *buf, int len)
{
 if (dt_ops.get_path)
  return dt_ops.get_path(phandle, buf, len);

 return ((void *)0);
}

static inline void *malloc(unsigned long size)
{
 return (platform_ops.malloc) ? platform_ops.malloc(size) : ((void *)0);
}

static inline void free(void *ptr)
{
 if (platform_ops.free)
  platform_ops.free(ptr);
}

static inline void exit(void)
{
 if (platform_ops.exit)
  platform_ops.exit();
 for(;;);
}







extern unsigned long timebase_period_ns;
void udelay(long delay);

extern char _start[];
extern char __bss_start[];
extern char _end[];
extern char _vmlinux_start[];
extern char _vmlinux_end[];
extern char _initrd_start[];
extern char _initrd_end[];
extern char _dtb_start[];
extern char _dtb_end[];

static inline __attribute__((const))
int __ilog2_u32(u32 n)
{
 int bit;
 asm ("cntlzw %0,%1" : "=r" (bit) : "r" (n));
 return 31 - bit;
}
# 19 "../include/flatdevtree_env.h" 2
# 21 "flatdevtree.h" 2
# 32 "flatdevtree.h"
struct boot_param_header {
 u32 magic;
 u32 totalsize;
 u32 off_dt_struct;
 u32 off_dt_strings;
 u32 off_mem_rsvmap;
 u32 version;
 u32 last_comp_version;

 u32 boot_cpuid_phys;

 u32 dt_strings_size;
};

struct ft_reserve {
 u64 start;
 u64 len;
};

struct ft_region {
 char *start;
 unsigned long size;
};

enum ft_rgn_id {
 FT_RSVMAP,
 FT_STRUCT,
 FT_STRINGS,
 FT_N_REGION
};



struct ft_cxt {
 struct boot_param_header *bph;
 int max_size;
 int isordered;
 void *(*realloc)(void *, unsigned long);
 char *str_anchor;
 char *p;
 struct ft_region rgn[FT_N_REGION];
 void *genealogy[50 +1];
 char **node_tbl;
 unsigned int node_max;
 unsigned int nodes_used;
};

char *ft_begin_node(struct ft_cxt *cxt, const char *name);
void ft_end_node(struct ft_cxt *cxt);

void ft_begin_tree(struct ft_cxt *cxt);
void ft_end_tree(struct ft_cxt *cxt);

void ft_nop(struct ft_cxt *cxt);
int ft_prop(struct ft_cxt *cxt, const char *name,
     const void *data, unsigned int sz);
int ft_prop_str(struct ft_cxt *cxt, const char *name, const char *str);
int ft_prop_int(struct ft_cxt *cxt, const char *name, unsigned int val);
void ft_begin(struct ft_cxt *cxt, void *blob, unsigned int max_size,
       void *(*realloc_fn)(void *, unsigned long));
int ft_open(struct ft_cxt *cxt, void *blob, unsigned int max_size,
  unsigned int max_find_device,
  void *(*realloc_fn)(void *, unsigned long));
int ft_add_rsvmap(struct ft_cxt *cxt, u64 physaddr, u64 size);

void ft_dump_blob(const void *bphp);
void ft_merge_blob(struct ft_cxt *cxt, void *blob);
void *ft_find_device(struct ft_cxt *cxt, const void *top,
                     const char *srch_path);
void *ft_find_descendent(struct ft_cxt *cxt, void *top, const char *srch_path);
int ft_get_prop(struct ft_cxt *cxt, const void *phandle, const char *propname,
  void *buf, const unsigned int buflen);
int ft_set_prop(struct ft_cxt *cxt, const void *phandle, const char *propname,
  const void *buf, const unsigned int buflen);
void *ft_get_parent(struct ft_cxt *cxt, const void *phandle);
void *ft_find_node_by_prop_value(struct ft_cxt *cxt, const void *prev,
                                 const char *propname, const char *propval,
                                 int proplen);
void *ft_create_node(struct ft_cxt *cxt, const void *parent, const char *name);
char *ft_get_path(struct ft_cxt *cxt, const void *phandle, char *buf, int len);
# 28 "flatdevtree.c" 2




static char *ft_root_node(struct ft_cxt *cxt)
{
 return cxt->rgn[FT_STRUCT].start;
}



static void *ft_get_phandle(struct ft_cxt *cxt, char *node)
{
 unsigned int i;

 if (!node)
  return ((void *)0);

 for (i = 1; i < cxt->nodes_used; i++)
  if (cxt->node_tbl[i] == node)
   return (void *)i;

 if (cxt->nodes_used < cxt->node_max) {
  cxt->node_tbl[cxt->nodes_used] = node;
  return (void *)cxt->nodes_used++;
 }

 return ((void *)0);
}

static char *ft_node_ph2node(struct ft_cxt *cxt, const void *phandle)
{
 unsigned int i = (unsigned int)phandle;

 if (i < cxt->nodes_used)
  return cxt->node_tbl[i];
 return ((void *)0);
}

static void ft_node_update_before(struct ft_cxt *cxt, char *addr, int shift)
{
 unsigned int i;

 if (shift == 0)
  return;

 for (i = 1; i < cxt->nodes_used; i++)
  if (cxt->node_tbl[i] < addr)
   cxt->node_tbl[i] += shift;
}

static void ft_node_update_after(struct ft_cxt *cxt, char *addr, int shift)
{
 unsigned int i;

 if (shift == 0)
  return;

 for (i = 1; i < cxt->nodes_used; i++)
  if (cxt->node_tbl[i] >= addr)
   cxt->node_tbl[i] += shift;
}


struct ft_atom {
 u32 tag;
 const char *name;
 void *data;
 u32 size;
};


static char *ft_next(struct ft_cxt *cxt, char *p, struct ft_atom *ret)
{
 u32 sz;

 if (p >= cxt->rgn[FT_STRUCT].start + cxt->rgn[FT_STRUCT].size)
  return ((void *)0);

 ret->tag = (*(u32 *) p);
 p += 4;

 switch (ret->tag) {
 case 0x1:
  ret->name = p;
  ret->data = (void *)(p - 4);
  p += (((strlen(p) + 1) + (4) - 1) & ~((4) - 1));
  break;
 case 0x3:
  ret->size = sz = (*(u32 *) p);
  ret->name = cxt->str_anchor + (*(u32 *) (p + 4));
  ret->data = (void *)(p + 8);
  p += 8 + (((sz) + (4) - 1) & ~((4) - 1));
  break;
 case 0x2:
 case 0x4:
  break;
 case 0x9:
 default:
  p = ((void *)0);
  break;
 }

 return p;
}





static int ft_reorder(struct ft_cxt *cxt, int nextra)
{
 unsigned long tot;
 enum ft_rgn_id r;
 char *p, *pend;
 int stroff;

 tot = (((sizeof(struct boot_param_header)) + (8) - 1) & ~((8) - 1)) + 1024;
 for (r = FT_RSVMAP; r <= FT_STRINGS; ++r)
  tot += cxt->rgn[r].size;
 if (nextra > 0)
  tot += nextra;
 tot = (((tot) + (8) - 1) & ~((8) - 1));

 if (!cxt->realloc)
  return 0;
 p = cxt->realloc(((void *)0), tot);
 if (!p)
  return 0;

 memcpy(p, cxt->bph, sizeof(struct boot_param_header));


 cxt->bph = (struct boot_param_header *)p;
 cxt->max_size = tot;
 pend = p + tot;
 p += (((sizeof(struct boot_param_header)) + (8) - 1) & ~((8) - 1));

 memcpy(p, cxt->rgn[FT_RSVMAP].start, cxt->rgn[FT_RSVMAP].size);
 cxt->rgn[FT_RSVMAP].start = p;
 p += cxt->rgn[FT_RSVMAP].size;

 memcpy(p, cxt->rgn[FT_STRUCT].start, cxt->rgn[FT_STRUCT].size);
 ft_node_update_after(cxt, cxt->rgn[FT_STRUCT].start,
   p - cxt->rgn[FT_STRUCT].start);
 cxt->p += p - cxt->rgn[FT_STRUCT].start;
 cxt->rgn[FT_STRUCT].start = p;

 p = pend - cxt->rgn[FT_STRINGS].size;
 memcpy(p, cxt->rgn[FT_STRINGS].start, cxt->rgn[FT_STRINGS].size);
 stroff = cxt->str_anchor - cxt->rgn[FT_STRINGS].start;
 cxt->rgn[FT_STRINGS].start = p;
 cxt->str_anchor = p + stroff;

 cxt->isordered = 1;
 return 1;
}

static inline char *prev_end(struct ft_cxt *cxt, enum ft_rgn_id r)
{
 if (r > FT_RSVMAP)
  return cxt->rgn[r - 1].start + cxt->rgn[r - 1].size;
 return (char *)cxt->bph + (((sizeof(struct boot_param_header)) + (8) - 1) & ~((8) - 1));
}

static inline char *next_start(struct ft_cxt *cxt, enum ft_rgn_id r)
{
 if (r < FT_STRINGS)
  return cxt->rgn[r + 1].start;
 return (char *)cxt->bph + cxt->max_size;
}





static int ft_shuffle(struct ft_cxt *cxt, char **pp, enum ft_rgn_id rgn,
  int nextra)
{
 char *p = *pp;
 char *rgn_start, *rgn_end;

 rgn_start = cxt->rgn[rgn].start;
 rgn_end = rgn_start + cxt->rgn[rgn].size;
 if (nextra <= 0 || rgn_end + nextra <= next_start(cxt, rgn)) {

  if (p < rgn_end) {
   if (nextra < 0)
    memmove(p, p - nextra, rgn_end - p + nextra);
   else
    memmove(p + nextra, p, rgn_end - p);
   if (rgn == FT_STRUCT)
    ft_node_update_after(cxt, p, nextra);
  }
  cxt->rgn[rgn].size += nextra;
  if (rgn == FT_STRINGS)

   cxt->str_anchor += nextra;
  return 1;
 }
 if (prev_end(cxt, rgn) <= rgn_start - nextra) {

  if (p > rgn_start) {
   memmove(rgn_start - nextra, rgn_start, p - rgn_start);
   if (rgn == FT_STRUCT)
    ft_node_update_before(cxt, p, -nextra);
  }
  *pp -= nextra;
  cxt->rgn[rgn].start -= nextra;
  cxt->rgn[rgn].size += nextra;
  return 1;
 }
 return 0;
}

static int ft_make_space(struct ft_cxt *cxt, char **pp, enum ft_rgn_id rgn,
    int nextra)
{
 unsigned long size, ssize, tot;
 char *str, *next;
 enum ft_rgn_id r;

 if (!cxt->isordered) {
  unsigned long rgn_off = *pp - cxt->rgn[rgn].start;

  if (!ft_reorder(cxt, nextra))
   return 0;

  *pp = cxt->rgn[rgn].start + rgn_off;
 }
 if (ft_shuffle(cxt, pp, rgn, nextra))
  return 1;


 ssize = cxt->rgn[FT_STRINGS].size;
 if (cxt->rgn[FT_STRINGS].start + ssize
   < (char *)cxt->bph + cxt->max_size) {

  str = (char *)cxt->bph + cxt->max_size - ssize;
  cxt->str_anchor += str - cxt->rgn[FT_STRINGS].start;
  memmove(str, cxt->rgn[FT_STRINGS].start, ssize);
  cxt->rgn[FT_STRINGS].start = str;

  if (rgn >= FT_STRUCT && ft_shuffle(cxt, pp, rgn, nextra))
   return 1;
 }


 tot = 0;
 for (r = rgn; r < FT_STRINGS; ++r) {
  char *r_end = cxt->rgn[r].start + cxt->rgn[r].size;
  tot += next_start(cxt, rgn) - r_end;
 }


 if (tot < (unsigned int)nextra) {

  char *newp, *new_start;
  int shift;

  if (!cxt->realloc)
   return 0;
  size = (((cxt->max_size + (nextra - tot) + 1024) + (8) - 1) & ~((8) - 1));
  newp = cxt->realloc(cxt->bph, size);
  if (!newp)
   return 0;
  cxt->max_size = size;
  shift = newp - (char *)cxt->bph;

  if (shift) {
   cxt->bph = (struct boot_param_header *)newp;
   ft_node_update_after(cxt, cxt->rgn[FT_STRUCT].start,
     shift);
   for (r = FT_RSVMAP; r <= FT_STRINGS; ++r) {
    new_start = cxt->rgn[r].start + shift;
    cxt->rgn[r].start = new_start;
   }
   *pp += shift;
   cxt->str_anchor += shift;
  }


  str = newp + size - ssize;
  cxt->str_anchor += str - cxt->rgn[FT_STRINGS].start;
  memmove(str, cxt->rgn[FT_STRINGS].start, ssize);
  cxt->rgn[FT_STRINGS].start = str;

  if (ft_shuffle(cxt, pp, rgn, nextra))
   return 1;
 }


 if (rgn == FT_RSVMAP) {
  next = cxt->rgn[FT_RSVMAP].start + cxt->rgn[FT_RSVMAP].size
   + nextra;
  ssize = cxt->rgn[FT_STRUCT].size;
  if (next + ssize >= cxt->rgn[FT_STRINGS].start)
   return 0;
  memmove(next, cxt->rgn[FT_STRUCT].start, ssize);
  ft_node_update_after(cxt, cxt->rgn[FT_STRUCT].start, nextra);
  cxt->rgn[FT_STRUCT].start = next;

  if (ft_shuffle(cxt, pp, rgn, nextra))
   return 1;
 }

 return 0;
}

static void ft_put_word(struct ft_cxt *cxt, u32 v)
{
 *(u32 *) cxt->p = (v);
 cxt->p += 4;
}

static void ft_put_bin(struct ft_cxt *cxt, const void *data, unsigned int sz)
{
 unsigned long sza = (((sz) + (4) - 1) & ~((4) - 1));


 if (sz < sza)
  *(u32 *) (cxt->p + sza - 4) = 0;


 memcpy(cxt->p, data, sz);

 cxt->p += sza;
}

char *ft_begin_node(struct ft_cxt *cxt, const char *name)
{
 unsigned long nlen = strlen(name) + 1;
 unsigned long len = 8 + (((nlen) + (4) - 1) & ~((4) - 1));
 char *ret;

 if (!ft_make_space(cxt, &cxt->p, FT_STRUCT, len))
  return ((void *)0);

 ret = cxt->p;

 ft_put_word(cxt, 0x1);
 ft_put_bin(cxt, name, strlen(name) + 1);

 return ret;
}

void ft_end_node(struct ft_cxt *cxt)
{
 ft_put_word(cxt, 0x2);
}

void ft_nop(struct ft_cxt *cxt)
{
 if (ft_make_space(cxt, &cxt->p, FT_STRUCT, 4))
  ft_put_word(cxt, 0x4);
}



static int lookup_string(struct ft_cxt *cxt, const char *name)
{
 char *p, *end;

 p = cxt->rgn[FT_STRINGS].start;
 end = p + cxt->rgn[FT_STRINGS].size;
 while (p < end) {
  if (strcmp(p, (char *)name) == 0)
   return p - cxt->str_anchor;
  p += strlen(p) + 1;
 }

 return 0x7fffffff;
}


static int map_string(struct ft_cxt *cxt, const char *name)
{
 int off;
 char *p;

 off = lookup_string(cxt, name);
 if (off != 0x7fffffff)
  return off;
 p = cxt->rgn[FT_STRINGS].start;
 if (!ft_make_space(cxt, &p, FT_STRINGS, strlen(name) + 1))
  return 0x7fffffff;
 strcpy(p, name);
 return p - cxt->str_anchor;
}

int ft_prop(struct ft_cxt *cxt, const char *name, const void *data,
  unsigned int sz)
{
 int off, len;

 off = map_string(cxt, name);
 if (off == 0x7fffffff)
  return -1;

 len = 12 + (((sz) + (4) - 1) & ~((4) - 1));
 if (!ft_make_space(cxt, &cxt->p, FT_STRUCT, len))
  return -1;

 ft_put_word(cxt, 0x3);
 ft_put_word(cxt, sz);
 ft_put_word(cxt, off);
 ft_put_bin(cxt, data, sz);
 return 0;
}

int ft_prop_str(struct ft_cxt *cxt, const char *name, const char *str)
{
 return ft_prop(cxt, name, str, strlen(str) + 1);
}

int ft_prop_int(struct ft_cxt *cxt, const char *name, unsigned int val)
{
 u32 v = ((u32) val);

 return ft_prop(cxt, name, &v, 4);
}


static unsigned long rsvmap_size(struct ft_cxt *cxt)
{
 struct ft_reserve *res;

 res = (struct ft_reserve *)cxt->rgn[FT_RSVMAP].start;
 while (res->start || res->len)
  ++res;
 return (char *)(res + 1) - cxt->rgn[FT_RSVMAP].start;
}


static unsigned long struct_size(struct ft_cxt *cxt)
{
 char *p = cxt->rgn[FT_STRUCT].start;
 char *next;
 struct ft_atom atom;


 if (cxt->rgn[FT_STRUCT].size == 0)
  cxt->rgn[FT_STRUCT].size = 0xfffffffful - (unsigned long)p;

 while ((next = ft_next(cxt, p, &atom)) != ((void *)0))
  p = next;
 return p + 4 - cxt->rgn[FT_STRUCT].start;
}


static void adjust_string_offsets(struct ft_cxt *cxt, int adj)
{
 char *p = cxt->rgn[FT_STRUCT].start;
 char *next;
 struct ft_atom atom;
 int off;

 while ((next = ft_next(cxt, p, &atom)) != ((void *)0)) {
  if (atom.tag == 0x3) {
   off = (*(u32 *) (p + 8));
   *(u32 *) (p + 8) = (off + adj);
  }
  p = next;
 }
}


void ft_begin(struct ft_cxt *cxt, void *blob, unsigned int max_size,
  void *(*realloc_fn) (void *, unsigned long))
{
 struct boot_param_header *bph = blob;
 char *p;
 struct ft_reserve *pres;


 memset(cxt, 0, sizeof(*cxt));

 cxt->bph = bph;
 cxt->max_size = max_size;
 cxt->realloc = realloc_fn;
 cxt->isordered = 1;


 memset(bph, 0, sizeof(*bph));

 bph->magic = (0xd00dfeed);
 bph->version = (0x10);
 bph->last_comp_version = (0x10);


 cxt->rgn[FT_RSVMAP].start = p = blob + (((sizeof(struct boot_param_header)) + (8) - 1) & ~((8) - 1));
 cxt->rgn[FT_RSVMAP].size = sizeof(struct ft_reserve);
 pres = (struct ft_reserve *)p;
 cxt->rgn[FT_STRUCT].start = p += sizeof(struct ft_reserve);
 cxt->rgn[FT_STRUCT].size = 4;
 cxt->rgn[FT_STRINGS].start = blob + max_size;
 cxt->rgn[FT_STRINGS].size = 0;


 pres->start = 0;
 pres->len = 0;
 *(u32 *) p = (0x9);

 cxt->str_anchor = blob;
}


int ft_open(struct ft_cxt *cxt, void *blob, unsigned int max_size,
  unsigned int max_find_device,
  void *(*realloc_fn) (void *, unsigned long))
{
 struct boot_param_header *bph = blob;


 if ((bph->version) < 16)
  return -1;


 memset(cxt, 0, sizeof(*cxt));


 ++max_find_device;
 cxt->node_tbl = realloc_fn(((void *)0), max_find_device * sizeof(char *));
 if (!cxt->node_tbl)
  return -1;
 memset(cxt->node_tbl, 0, max_find_device * sizeof(char *));
 cxt->node_max = max_find_device;
 cxt->nodes_used = 1;

 cxt->bph = bph;
 cxt->max_size = max_size;
 cxt->realloc = realloc_fn;

 cxt->rgn[FT_RSVMAP].start = blob + (bph->off_mem_rsvmap);
 cxt->rgn[FT_RSVMAP].size = rsvmap_size(cxt);
 cxt->rgn[FT_STRUCT].start = blob + (bph->off_dt_struct);
 cxt->rgn[FT_STRUCT].size = struct_size(cxt);
 cxt->rgn[FT_STRINGS].start = blob + (bph->off_dt_strings);
 cxt->rgn[FT_STRINGS].size = (bph->dt_strings_size);

 cxt->p = cxt->rgn[FT_STRUCT].start;
 cxt->str_anchor = cxt->rgn[FT_STRINGS].start;

 return 0;
}


int ft_add_rsvmap(struct ft_cxt *cxt, u64 physaddr, u64 size)
{
 char *p;
 struct ft_reserve *pres;

 p = cxt->rgn[FT_RSVMAP].start + cxt->rgn[FT_RSVMAP].size
  - sizeof(struct ft_reserve);
 if (!ft_make_space(cxt, &p, FT_RSVMAP, sizeof(struct ft_reserve)))
  return -1;

 pres = (struct ft_reserve *)p;
 pres->start = (physaddr);
 pres->len = (size);

 return 0;
}

void ft_begin_tree(struct ft_cxt *cxt)
{
 cxt->p = ft_root_node(cxt);
}

void ft_end_tree(struct ft_cxt *cxt)
{
 struct boot_param_header *bph = cxt->bph;
 char *p, *oldstr, *str, *endp;
 unsigned long ssize;
 int adj;

 if (!cxt->isordered)
  return;


 oldstr = cxt->rgn[FT_STRINGS].start;
 adj = cxt->str_anchor - oldstr;
 if (adj)
  adjust_string_offsets(cxt, adj);


 ssize = cxt->rgn[FT_STRINGS].size;
 endp = (char *)((((unsigned long)cxt->rgn[FT_STRUCT].start + cxt->rgn[FT_STRUCT].size + ssize) + (8) - 1) & ~((8) - 1));

 str = endp - ssize;


 memmove(str, oldstr, ssize);
 cxt->str_anchor = str;
 cxt->rgn[FT_STRINGS].start = str;


 p = (char *)bph;
 bph->totalsize = (endp - p);
 bph->off_mem_rsvmap = (cxt->rgn[FT_RSVMAP].start - p);
 bph->off_dt_struct = (cxt->rgn[FT_STRUCT].start - p);
 bph->off_dt_strings = (cxt->rgn[FT_STRINGS].start - p);
 bph->dt_strings_size = (ssize);
}

void *ft_find_device(struct ft_cxt *cxt, const void *top, const char *srch_path)
{
 char *node;

 if (top) {
  node = ft_node_ph2node(cxt, top);
  if (node == ((void *)0))
   return ((void *)0);
 } else {
  node = ft_root_node(cxt);
 }

 node = ft_find_descendent(cxt, node, srch_path);
 return ft_get_phandle(cxt, node);
}

void *ft_find_descendent(struct ft_cxt *cxt, void *top, const char *srch_path)
{
 struct ft_atom atom;
 char *p;
 const char *cp, *q;
 int cl;
 int depth = -1;
 int dmatch = 0;
 const char *path_comp[50];

 cp = srch_path;
 cl = 0;
 p = top;

 while ((p = ft_next(cxt, p, &atom)) != ((void *)0)) {
  switch (atom.tag) {
  case 0x1:
   ++depth;
   if (depth != dmatch)
    break;
   cxt->genealogy[depth] = atom.data;
   cxt->genealogy[depth + 1] = ((void *)0);
   if (depth && !(strncmp(atom.name, cp, cl) == 0
     && (atom.name[cl] == '/'
      || atom.name[cl] == '\0'
      || atom.name[cl] == '@')))
    break;
   path_comp[dmatch] = cp;

   cp += cl;

   while (*cp == '/')
    ++cp;

   if (*cp == 0)
    return atom.data;

   q = strchr(cp, '/');
   if (q)
    cl = q - cp;
   else
    cl = strlen(cp);
   ++dmatch;
   break;
  case 0x2:
   if (depth == 0)
    return ((void *)0);
   if (dmatch > depth) {
    --dmatch;
    cl = cp - path_comp[dmatch] - 1;
    cp = path_comp[dmatch];
    while (cl > 0 && cp[cl - 1] == '/')
     --cl;
   }
   --depth;
   break;
  }
 }
 return ((void *)0);
}

void *__ft_get_parent(struct ft_cxt *cxt, void *node)
{
 int d;
 struct ft_atom atom;
 char *p;

 for (d = 0; cxt->genealogy[d] != ((void *)0); ++d)
  if (cxt->genealogy[d] == node)
   return d > 0 ? cxt->genealogy[d - 1] : ((void *)0);


 p = ft_root_node(cxt);
 d = 0;
 while ((p = ft_next(cxt, p, &atom)) != ((void *)0)) {
  switch (atom.tag) {
  case 0x1:
   cxt->genealogy[d] = atom.data;
   if (node == atom.data) {

    cxt->genealogy[d + 1] = ((void *)0);
    return d > 0 ? cxt->genealogy[d - 1] : ((void *)0);
   }
   ++d;
   break;
  case 0x2:
   --d;
   break;
  }
 }
 return ((void *)0);
}

void *ft_get_parent(struct ft_cxt *cxt, const void *phandle)
{
 void *node = ft_node_ph2node(cxt, phandle);
 if (node == ((void *)0))
  return ((void *)0);

 node = __ft_get_parent(cxt, node);
 return ft_get_phandle(cxt, node);
}

static const void *__ft_get_prop(struct ft_cxt *cxt, void *node,
                                 const char *propname, unsigned int *len)
{
 struct ft_atom atom;
 int depth = 0;

 while ((node = ft_next(cxt, node, &atom)) != ((void *)0)) {
  switch (atom.tag) {
  case 0x1:
   ++depth;
   break;

  case 0x3:
   if (depth != 1 || strcmp(atom.name, propname))
    break;

   if (len)
    *len = atom.size;

   return atom.data;

  case 0x2:
   if (--depth <= 0)
    return ((void *)0);
  }
 }

 return ((void *)0);
}

int ft_get_prop(struct ft_cxt *cxt, const void *phandle, const char *propname,
  void *buf, const unsigned int buflen)
{
 const void *data;
 unsigned int size;

 void *node = ft_node_ph2node(cxt, phandle);
 if (!node)
  return -1;

 data = __ft_get_prop(cxt, node, propname, &size);
 if (data) {
  unsigned int clipped_size = ({ typeof(size) _x = (size); typeof(buflen) _y = (buflen); (void) (&_x == &_y); _x < _y ? _x : _y; });
  memcpy(buf, data, clipped_size);
  return size;
 }

 return -1;
}

void *__ft_find_node_by_prop_value(struct ft_cxt *cxt, void *prev,
                                   const char *propname, const char *propval,
                                   unsigned int proplen)
{
 struct ft_atom atom;
 char *p = ft_root_node(cxt);
 char *next;
 int past_prev = prev ? 0 : 1;
 int depth = -1;

 while ((next = ft_next(cxt, p, &atom)) != ((void *)0)) {
  const void *data;
  unsigned int size;

  switch (atom.tag) {
  case 0x1:
   depth++;

   if (prev == p) {
    past_prev = 1;
    break;
   }

   if (!past_prev || depth < 1)
    break;

   data = __ft_get_prop(cxt, p, propname, &size);
   if (!data || size != proplen)
    break;
   if (memcmp(data, propval, size))
    break;

   return p;

  case 0x2:
   if (depth-- == 0)
    return ((void *)0);

   break;
  }

  p = next;
 }

 return ((void *)0);
}

void *ft_find_node_by_prop_value(struct ft_cxt *cxt, const void *prev,
                                 const char *propname, const char *propval,
                                 int proplen)
{
 void *node = ((void *)0);

 if (prev) {
  node = ft_node_ph2node(cxt, prev);

  if (!node)
   return ((void *)0);
 }

 node = __ft_find_node_by_prop_value(cxt, node, propname,
                                     propval, proplen);
 return ft_get_phandle(cxt, node);
}

int ft_set_prop(struct ft_cxt *cxt, const void *phandle, const char *propname,
  const void *buf, const unsigned int buflen)
{
 struct ft_atom atom;
 void *node;
 char *p, *next;
 int nextra;

 node = ft_node_ph2node(cxt, phandle);
 if (node == ((void *)0))
  return -1;

 next = ft_next(cxt, node, &atom);
 if (atom.tag != 0x1)

  return -1;
 p = next;

 while ((next = ft_next(cxt, p, &atom)) != ((void *)0)) {
  switch (atom.tag) {
  case 0x1:
  case 0x2:

   cxt->p = p;
   return ft_prop(cxt, propname, buf, buflen);
  case 0x3:
   if (strcmp(atom.name, propname))
    break;

   nextra = (((buflen) + (4) - 1) & ~((4) - 1)) - (((atom.size) + (4) - 1) & ~((4) - 1));
   cxt->p = atom.data;
   if (nextra && !ft_make_space(cxt, &cxt->p, FT_STRUCT,
      nextra))
    return -1;
   *(u32 *) (cxt->p - 8) = (buflen);
   ft_put_bin(cxt, buf, buflen);
   return 0;
  }
  p = next;
 }
 return -1;
}

int ft_del_prop(struct ft_cxt *cxt, const void *phandle, const char *propname)
{
 struct ft_atom atom;
 void *node;
 char *p, *next;
 int size;

 node = ft_node_ph2node(cxt, phandle);
 if (node == ((void *)0))
  return -1;

 p = node;
 while ((next = ft_next(cxt, p, &atom)) != ((void *)0)) {
  switch (atom.tag) {
  case 0x1:
  case 0x2:
   return -1;
  case 0x3:
   if (strcmp(atom.name, propname))
    break;

   size = 12 + -(((atom.size) + (4) - 1) & ~((4) - 1));
   cxt->p = p;
   if (!ft_make_space(cxt, &cxt->p, FT_STRUCT, -size))
    return -1;
   return 0;
  }
  p = next;
 }
 return -1;
}

void *ft_create_node(struct ft_cxt *cxt, const void *parent, const char *name)
{
 struct ft_atom atom;
 char *p, *next, *ret;
 int depth = 0;

 if (parent) {
  p = ft_node_ph2node(cxt, parent);
  if (!p)
   return ((void *)0);
 } else {
  p = ft_root_node(cxt);
 }

 while ((next = ft_next(cxt, p, &atom)) != ((void *)0)) {
  switch (atom.tag) {
  case 0x1:
   ++depth;
   if (depth == 1 && strcmp(atom.name, name) == 0)

    return ((void *)0);
   break;
  case 0x2:
   --depth;
   if (depth > 0)
    break;

   cxt->p = p;
   ret = ft_begin_node(cxt, name);
   ft_end_node(cxt);
   return ft_get_phandle(cxt, ret);
  }
  p = next;
 }
 return ((void *)0);
}




char *ft_get_path(struct ft_cxt *cxt, const void *phandle,
                  char *buf, int len)
{
 const char *path_comp[50];
 struct ft_atom atom;
 char *p, *next, *pos;
 int depth = 0, i;
 void *node;

 node = ft_node_ph2node(cxt, phandle);
 if (node == ((void *)0))
  return ((void *)0);

 p = ft_root_node(cxt);

 while ((next = ft_next(cxt, p, &atom)) != ((void *)0)) {
  switch (atom.tag) {
  case 0x1:
   path_comp[depth++] = atom.name;
   if (p == node)
    goto found;

   break;

  case 0x2:
   if (--depth == 0)
    return ((void *)0);
  }

  p = next;
 }

found:
 pos = buf;
 for (i = 1; i < depth; i++) {
  int this_len;

  if (len <= 1)
   return ((void *)0);

  *pos++ = '/';
  len--;

  strncpy(pos, path_comp[i], len);

  if (pos[len - 1] != 0)
   return ((void *)0);

  this_len = strlen(pos);
  len -= this_len;
  pos += this_len;
 }

 return buf;
}
