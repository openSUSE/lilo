/* $Id$ */
#include <stdio.h>

/* #include <linux/keyboard.h> */
#define KG_SHIFT	0
#define KG_CTRL		2
#define KG_ALT		3
#define KG_ALTGR	1
#define KG_SHIFTL	4
#define KG_KANASHIFT	4
#define KG_SHIFTR	5
#define KG_CTRLL	6
#define KG_CTRLR	7
#define KG_CAPSSHIFT	8

#define NR_SHIFT	9

#define NR_KEYS		256

#define KT_LATIN	0	/* we depend on this being zero */
#define KT_LETTER	11	/* symbol that can be acted upon by CapsLock */
#define KT_FN		1
#define KT_SPEC		2
#define KT_PAD		3
#define KT_DEAD		4
#define KT_CONS		5
#define KT_CUR		6
#define KT_SHIFT	7
#define KT_META		8
#define KT_ASCII	9
#define KT_LOCK		10
#define KT_SLOCK	12

#define K(t,v)		(((t)<<8)|(v))
#define KTYP(x)		((x) >> 8)
#define KVAL(x)		((x) & 0xff)

#define K_ENTER		K(KT_SPEC,1)
#define K_CAPS		K(KT_SPEC,7)
#define K_PENTER	K(KT_PAD,14)	/* key-pad enter */

/* #include "defkeymap.c"	yeah I know it's bad -- Cort */
static unsigned short plain_map[NR_KEYS] = {
	0xf200,	0xf01b,	0xf031,	0xf032,	0xf033,	0xf034,	0xf035,	0xf036,
	0xf037,	0xf038,	0xf039,	0xf030,	0xf02d,	0xf03d,	0xf07f,	0xf009,
	0xfb71,	0xfb77,	0xfb65,	0xfb72,	0xfb74,	0xfb79,	0xfb75,	0xfb69,
	0xfb6f,	0xfb70,	0xf05b,	0xf05d,	0xf201,	0xf702,	0xfb61,	0xfb73,
	0xfb64,	0xfb66,	0xfb67,	0xfb68,	0xfb6a,	0xfb6b,	0xfb6c,	0xf03b,
	0xf027,	0xf060,	0xf700,	0xf05c,	0xfb7a,	0xfb78,	0xfb63,	0xfb76,
	0xfb62,	0xfb6e,	0xfb6d,	0xf02c,	0xf02e,	0xf02f,	0xf700,	0xf30c,
	0xf703,	0xf020,	0xf207,	0xf100,	0xf101,	0xf102,	0xf103,	0xf104,
	0xf105,	0xf106,	0xf107,	0xf108,	0xf109,	0xf208,	0xf209,	0xf307,
	0xf308,	0xf309,	0xf30b,	0xf304,	0xf305,	0xf306,	0xf30a,	0xf301,
	0xf302,	0xf303,	0xf300,	0xf310,	0xf206,	0xf200,	0xf03c,	0xf10a,
	0xf10b,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf30e,	0xf702,	0xf30d,	0xf01c,	0xf701,	0xf205,	0xf114,	0xf603,
	0xf118,	0xf601,	0xf602,	0xf117,	0xf600,	0xf119,	0xf115,	0xf116,
	0xf11a,	0xf10c,	0xf10d,	0xf11b,	0xf11c,	0xf110,	0xf311,	0xf11d,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
};

static unsigned short shift_map[NR_KEYS] = {
	0xf200,	0xf01b,	0xf021,	0xf040,	0xf023,	0xf024,	0xf025,	0xf05e,
	0xf026,	0xf02a,	0xf028,	0xf029,	0xf05f,	0xf02b,	0xf07f,	0xf009,
	0xfb51,	0xfb57,	0xfb45,	0xfb52,	0xfb54,	0xfb59,	0xfb55,	0xfb49,
	0xfb4f,	0xfb50,	0xf07b,	0xf07d,	0xf201,	0xf702,	0xfb41,	0xfb53,
	0xfb44,	0xfb46,	0xfb47,	0xfb48,	0xfb4a,	0xfb4b,	0xfb4c,	0xf03a,
	0xf022,	0xf07e,	0xf700,	0xf07c,	0xfb5a,	0xfb58,	0xfb43,	0xfb56,
	0xfb42,	0xfb4e,	0xfb4d,	0xf03c,	0xf03e,	0xf03f,	0xf700,	0xf30c,
	0xf703,	0xf020,	0xf207,	0xf10a,	0xf10b,	0xf10c,	0xf10d,	0xf10e,
	0xf10f,	0xf110,	0xf111,	0xf112,	0xf113,	0xf213,	0xf203,	0xf307,
	0xf308,	0xf309,	0xf30b,	0xf304,	0xf305,	0xf306,	0xf30a,	0xf301,
	0xf302,	0xf303,	0xf300,	0xf310,	0xf206,	0xf200,	0xf03e,	0xf10a,
	0xf10b,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf30e,	0xf702,	0xf30d,	0xf200,	0xf701,	0xf205,	0xf114,	0xf603,
	0xf20b,	0xf601,	0xf602,	0xf117,	0xf600,	0xf20a,	0xf115,	0xf116,
	0xf11a,	0xf10c,	0xf10d,	0xf11b,	0xf11c,	0xf110,	0xf311,	0xf11d,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
};

static unsigned short ctrl_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf200,	0xf000,	0xf01b,	0xf01c,	0xf01d,	0xf01e,
	0xf01f,	0xf07f,	0xf200,	0xf200,	0xf01f,	0xf200,	0xf008,	0xf200,
	0xf011,	0xf017,	0xf005,	0xf012,	0xf014,	0xf019,	0xf015,	0xf009,
	0xf00f,	0xf010,	0xf01b,	0xf01d,	0xf201,	0xf702,	0xf001,	0xf013,
	0xf004,	0xf006,	0xf007,	0xf008,	0xf00a,	0xf00b,	0xf00c,	0xf200,
	0xf007,	0xf000,	0xf700,	0xf01c,	0xf01a,	0xf018,	0xf003,	0xf016,
	0xf002,	0xf00e,	0xf00d,	0xf200,	0xf20e,	0xf07f,	0xf700,	0xf30c,
	0xf703,	0xf000,	0xf207,	0xf100,	0xf101,	0xf102,	0xf103,	0xf104,
	0xf105,	0xf106,	0xf107,	0xf108,	0xf109,	0xf208,	0xf204,	0xf307,
	0xf308,	0xf309,	0xf30b,	0xf304,	0xf305,	0xf306,	0xf30a,	0xf301,
	0xf302,	0xf303,	0xf300,	0xf310,	0xf206,	0xf200,	0xf200,	0xf10a,
	0xf10b,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf30e,	0xf702,	0xf30d,	0xf01c,	0xf701,	0xf205,	0xf114,	0xf603,
	0xf118,	0xf601,	0xf602,	0xf117,	0xf600,	0xf119,	0xf115,	0xf116,
	0xf11a,	0xf10c,	0xf10d,	0xf11b,	0xf11c,	0xf110,	0xf311,	0xf11d,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
};


static unsigned char shfts, ctls, alts, caps;

#define	KBDATAP		0x60	/* kbd data port */
#define	KBSTATUSPORT	0x61	/* kbd status */
#define	KBSTATP		0x64	/* kbd status port */
#define	KBINRDY		0x01
#define	KBOUTRDY	0x02

extern unsigned char inb(int port);
extern void outb(int port, char val);
extern void udelay(long x);

static int kbd(int noblock)
{
	unsigned char dt, brk, val;
	unsigned code;
loop:
	if (noblock) {
	    if ((inb(KBSTATP) & KBINRDY) == 0)
		return (-1);
	} else while((inb(KBSTATP) & KBINRDY) == 0) ;

	dt = inb(KBDATAP);

	brk = dt & 0x80;	/* brk == 1 on key release */
	dt = dt & 0x7f;		/* keycode */

	if (shfts)
	    code = shift_map[dt];
	else if (ctls)
	    code = ctrl_map[dt];
	else
	    code = plain_map[dt];

	val = KVAL(code);
	switch (KTYP(code) & 0x0f) {
	    case KT_LATIN:
		if (brk)
		    break;
		if (alts)
		    val |= 0x80;
		if (val == 0x7f)	/* map delete to backspace */
		    val = '\b';
		return val;

	    case KT_LETTER:
		if (brk)
		    break;
		if (caps)
		    val -= 'a'-'A';
		return val;

	    case KT_SPEC:
		if (brk)
		    break;
		if (val == KVAL(K_CAPS))
		    caps = !caps;
		else if (val == KVAL(K_ENTER)) {
enter:		    /* Wait for key up */
		    while (1) {
			while((inb(KBSTATP) & KBINRDY) == 0) ;
			dt = inb(KBDATAP);
			if (dt & 0x80) /* key up */ break;
		    }
		    return 10;
		}
		break;

	    case KT_PAD:
		if (brk)
		    break;
		if (val < 10)
		    return val;
		if (val == KVAL(K_PENTER))
		    goto enter;
		break;

	    case KT_SHIFT:
		switch (val) {
		    case KG_SHIFT:
		    case KG_SHIFTL:
		    case KG_SHIFTR:
			shfts = brk ? 0 : 1;
			break;
		    case KG_ALT:
		    case KG_ALTGR:
			alts = brk ? 0 : 1;
			break;
		    case KG_CTRL:
		    case KG_CTRLL:
		    case KG_CTRLR:
			ctls = brk ? 0 : 1;
			break;
		}
		break;

	    case KT_LOCK:
		switch (val) {
		    case KG_SHIFT:
		    case KG_SHIFTL:
		    case KG_SHIFTR:
			if (brk)
			    shfts = !shfts;
			break;
		    case KG_ALT:
		    case KG_ALTGR:
			if (brk)
			    alts = !alts;
			break;
		    case KG_CTRL:
		    case KG_CTRLL:
		    case KG_CTRLR:
			if (brk)
			    ctls = !ctls;
			break;
		}
		break;
	}
	if (brk) return (-1);  /* Ignore initial 'key up' codes */
	goto loop;
}

static int __kbdreset(void)
{
	unsigned char c;
	int i, t;

	/* flush input queue */
	t = 2000;
	while ((inb(KBSTATP) & KBINRDY))
	{
		(void)inb(KBDATAP);
		if (--t == 0)
			return 1;
	}
	/* Send self-test */
	t = 20000;
	while (inb(KBSTATP) & KBOUTRDY)
		if (--t == 0)
			return 2;
	outb(KBSTATP,0xAA);
	t = 200000;
	while ((inb(KBSTATP) & KBINRDY) == 0)	/* wait input ready */
		if (--t == 0)
			return 3;
	if ((c = inb(KBDATAP)) != 0x55)
	{
		printf("Keyboard self test failed - result: %02x", c);
	}
	/* Enable interrupts and keyboard controller */
	t = 20000;
	while (inb(KBSTATP) & KBOUTRDY)
		if (--t == 0) return 4;
	outb(KBSTATP,0x60);
	t = 20000;
	while (inb(KBSTATP) & KBOUTRDY)
		if (--t == 0) return 5;
	outb(KBDATAP,0x45);
	for (i = 0;  i < 10000;  i++) udelay(1);

	t = 20000;
	while (inb(KBSTATP) & KBOUTRDY)
		if (--t == 0) return 6;
	outb(KBSTATP,0x20);
	t = 200000;
	while ((inb(KBSTATP) & KBINRDY) == 0)	/* wait input ready */
		if (--t == 0) return 7;
	if (! (inb(KBDATAP) & 0x40)) {
		/*
		 * Quote from PS/2 System Reference Manual:
		 *
		 * "Address hex 0060 and address hex 0064 should be
		 * written only when the input-buffer-full bit and
		 * output-buffer-full bit in the Controller Status
		 * register are set 0." (KBINRDY and KBOUTRDY)
		 */
		t = 200000;
		while (inb(KBSTATP) & (KBINRDY | KBOUTRDY))
			if (--t == 0) return 8;
		outb(KBDATAP,0xF0);
		t = 200000;
		while (inb(KBSTATP) & (KBINRDY | KBOUTRDY))
			if (--t == 0) return 9;
		outb(KBDATAP,0x01);
	}
	t = 20000;
	while (inb(KBSTATP) & KBOUTRDY)
		if (--t == 0) return 10;
	outb(KBSTATP,0xAE);
	return 0;
}

static void kbdreset(void)
{
	int ret = __kbdreset();

	if (ret)
		printf("__kbdreset failed: %x", ret);
}

/* We have to actually read the keyboard when CRT_tstc is called,
 * since the pending data might be a key release code, and therefore
 * not valid data.  In this case, kbd() will return -1, even though there's
 * data to be read.  Of course, we might actually read a valid key press,
 * in which case it gets queued into key_pending for use by CRT_getc.
 */

static int kbd_reset;

static int key_pending = -1;

int CRT_getc(void)
{
	int c;
	if (!kbd_reset) {kbdreset(); kbd_reset++; }

        if (key_pending != -1) {
                c = key_pending;
                key_pending = -1;
                return c;
        } else {
	while ((c = kbd(0)) == 0) ;
                return c;
        }
}

int CRT_tstc(void)
{
	if (!kbd_reset) {kbdreset(); kbd_reset++; }

        while (key_pending == -1 && ((inb(KBSTATP) & KBINRDY) != 0)) {
                key_pending = kbd(1);
        }

        return (key_pending != -1);
}
