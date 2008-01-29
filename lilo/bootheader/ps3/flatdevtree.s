	.file	"flatdevtree.c"
	.section	".got2","aw"
.LCTOC1 = .+32768
	.section	".text"
	.align 2
	.type	ft_get_phandle, @function
ft_get_phandle:
	cmpwi 0,4,0
	stwu 1,-16(1)
	mr 10,3
	beq 0,.L2
	lwz 8,260(3)
	li 3,1
	cmplwi 7,8,1
	mtctr 8
	bge+ 7,.L4
	mtctr 3
	b .L4
.L5:
	lwz 9,252(10)
	lwzx 0,9,11
	cmpw 7,0,4
	beq 7,.L8
	addi 3,3,1
.L4:
	slwi 11,3,2
	bdnz .L5
	lwz 0,256(10)
	cmplw 7,8,0
	bge 7,.L2
	lwz 11,252(10)
	lwz 3,260(10)
	slwi 9,8,2
	addi 0,3,1
	stw 0,260(10)
	stwx 4,11,9
	b .L8
.L2:
	li 3,0
.L8:
	addi 1,1,16
	blr
	.size	ft_get_phandle,.-ft_get_phandle
	.align 2
	.type	ft_node_ph2node, @function
ft_node_ph2node:
	stwu 1,-16(1)
	slwi 11,4,2
	li 9,0
	lwz 0,260(3)
	cmplw 7,4,0
	bge 7,.L18
	lwz 9,252(3)
	lwzx 9,9,11
.L18:
	mr 3,9
	addi 1,1,16
	blr
	.size	ft_node_ph2node,.-ft_node_ph2node
	.align 2
	.type	ft_node_update_after, @function
ft_node_update_after:
	cmpwi 0,5,0
	stwu 1,-16(1)
	beq 0,.L27
	lwz 9,260(3)
	li 7,1
	cmplwi 7,9,1
	bge+ 7,.L30
	li 9,1
	b .L30
.L24:
	lwz 11,252(3)
	lwzx 0,11,8
	cmplw 7,0,4
	add 10,0,5
	blt 7,.L30
	stwx 10,11,8
.L30:
	addic. 9,9,-1
	slwi 8,7,2
	addi 7,7,1
	bne 0,.L24
.L27:
	addi 1,1,16
	blr
	.size	ft_node_update_after,.-ft_node_update_after
	.align 2
	.type	next_start, @function
next_start:
	slwi 0,4,3
	cmplwi 7,4,1
	stwu 1,-16(1)
	add 9,0,3
	bgt 7,.L32
	lwz 3,32(9)
	b .L34
.L32:
	lwz 9,4(3)
	lwz 0,0(3)
	add 3,0,9
.L34:
	addi 1,1,16
	blr
	.size	next_start,.-next_start
	.align 2
	.globl ft_end_node
	.type	ft_end_node, @function
ft_end_node:
	stwu 1,-16(1)
	lwz 9,20(3)
	addi 0,9,4
	stw 0,20(3)
	li 0,2
	stw 0,0(9)
	addi 1,1,16
	blr
	.size	ft_end_node,.-ft_end_node
	.align 2
	.type	ft_next, @function
ft_next:
	stwu 1,-16(1)
	mflr 0
	bcl 20,31,.LCF5
.LCF5:
	stw 0,20(1)
	stmw 30,8(1)
	mflr 30
	lwz 0,32(3)
	lwz 9,36(3)
	addis 30,30,.LCTOC1-.LCF5@ha
	addi 30,30,.LCTOC1-.LCF5@l
	add 0,0,9
	cmplw 7,4,0
	bge 7,.L39
	lwz 0,0(4)
	addi 31,4,4
	stw 0,0(5)
	cmpwi 7,0,2
	beq 7,.L42
	cmplwi 7,0,2
	bgt 7,.L44
	cmpwi 7,0,1
	bne 7,.L39
	b .L41
.L44:
	cmpwi 7,0,3
	beq 7,.L43
	cmpwi 7,0,4
	bne 7,.L39
	b .L42
.L41:
	stw 31,4(5)
	stw 4,8(5)
	mr 3,31
	bl strlen+32768@plt
	addi 3,3,4
	rlwinm 3,3,0,0,29
	add 31,31,3
	b .L42
.L43:
	lwz 9,4(4)
	lwz 11,16(3)
	addi 0,4,12
	stw 0,8(5)
	stw 9,12(5)
	addi 9,9,3
	rlwinm 9,9,0,0,29
	add 9,31,9
	lwz 0,4(31)
	addi 31,9,8
	add 11,11,0
	stw 11,4(5)
	b .L42
.L39:
	li 31,0
.L42:
	lwz 0,20(1)
	mr 3,31
	lmw 30,8(1)
	addi 1,1,16
	mtlr 0
	blr
	.size	ft_next,.-ft_next
	.align 2
	.type	__ft_get_prop, @function
__ft_get_prop:
	stwu 1,-48(1)
	mflr 0
	bcl 20,31,.LCF6
.LCF6:
	stw 0,52(1)
	stmw 26,24(1)
	mflr 30
	mr 26,3
	mr 29,4
	addis 30,30,.LCTOC1-.LCF6@ha
	mr 27,5
	addi 30,30,.LCTOC1-.LCF6@l
	mr 28,6
	li 31,0
	b .L64
.L48:
	lwz 0,8(1)
	cmpwi 7,0,2
	cmpwi 1,0,3
	beq 7,.L50
	cmpwi 7,0,1
	cmpwi 6,31,1
	beq 1,.L51
	bne 7,.L64
	addi 31,31,1
	b .L64
.L51:
	mr 4,27
	bne 6,.L64
	lwz 3,12(1)
	bl strcmp+32768@plt
	cmpwi 7,3,0
	bne 7,.L64
	cmpwi 7,28,0
	beq 7,.L54
	lwz 0,20(1)
	stw 0,0(28)
.L54:
	lwz 3,16(1)
	b .L56
.L50:
	addic. 31,31,-1
	ble 0,.L57
.L64:
	mr 4,29
	addi 5,1,8
	mr 3,26
	bl ft_next@local
	mr. 29,3
	bne 0,.L48
.L57:
	li 3,0
.L56:
	lwz 0,52(1)
	lmw 26,24(1)
	addi 1,1,48
	mtlr 0
	blr
	.size	__ft_get_prop,.-__ft_get_prop
	.align 2
	.globl ft_end_tree
	.type	ft_end_tree, @function
ft_end_tree:
	stwu 1,-64(1)
	mflr 0
	bcl 20,31,.LCF7
.LCF7:
	stw 0,68(1)
	stmw 25,36(1)
	mflr 30
	mr 31,3
	lwz 0,8(3)
	lwz 26,0(3)
	addis 30,30,.LCTOC1-.LCF7@ha
	addi 30,30,.LCTOC1-.LCF7@l
	cmpwi 7,0,0
	beq 7,.L74
	lwz 25,40(3)
	lwz 0,16(3)
	subf. 28,25,0
	beq 0,.L68
	lwz 29,32(3)
	b .L70
.L71:
	lwz 0,8(1)
	cmpwi 7,0,3
	bne 7,.L72
	lwz 0,8(29)
	add 0,0,28
	stw 0,8(29)
.L72:
	mr 29,3
.L70:
	mr 4,29
	mr 3,31
	addi 5,1,8
	bl ft_next@local
	cmpwi 0,3,0
	bne 0,.L71
.L68:
	lwz 27,44(31)
	lwz 29,36(31)
	mr 4,25
	lwz 9,32(31)
	add 29,27,29
	mr 5,27
	addi 9,9,7
	add 29,29,9
	rlwinm 29,29,0,0,28
	subf 28,27,29
	subf 29,26,29
	mr 3,28
	bl memmove+32768@plt
	lwz 0,32(31)
	lwz 9,24(31)
	subf 11,26,28
	stw 27,32(26)
	stw 29,4(26)
	subf 9,26,9
	subf 0,26,0
	stw 11,12(26)
	stw 28,16(31)
	stw 0,8(26)
	stw 9,16(26)
	stw 28,40(31)
.L74:
	lwz 0,68(1)
	lmw 25,36(1)
	addi 1,1,64
	mtlr 0
	blr
	.size	ft_end_tree,.-ft_end_tree
	.align 2
	.type	ft_shuffle, @function
ft_shuffle:
	stwu 1,-48(1)
	slwi 9,5,3
	mflr 0
	bcl 20,31,.LCF8
.LCF8:
	add 9,9,3
	stw 0,52(1)
	mfcr 12
	cmpwi 4,6,0
	stw 12,12(1)
	stmw 24,16(1)
	mflr 30
	mr 26,5
	mr 27,3
	lwz 0,28(9)
	lwz 25,24(9)
	addis 30,30,.LCTOC1-.LCF8@ha
	mr 24,4
	addi 30,30,.LCTOC1-.LCF8@l
	mr 28,6
	lwz 29,0(4)
	add 31,25,0
	ble 4,.L76
	mr 4,5
	bl next_start@local
	add 0,31,28
	cmplw 7,0,3
	bgt 7,.L78
.L76:
	cmplw 7,29,31
	bge 7,.L79
	subf 5,29,31
	bge 4,.L81
	add 5,5,28
	mr 3,29
	subf 4,28,29
	b .L103
.L81:
	add 3,29,28
	mr 4,29
.L103:
	bl memmove+32768@plt
	cmpwi 7,26,1
	bne 7,.L79
	mr 4,29
	mr 3,27
	mr 5,28
	bl ft_node_update_after@local
	lwz 0,36(27)
	li 11,1
	add 0,0,28
	stw 0,36(27)
	b .L85
.L79:
	slwi 9,26,3
	add 9,9,27
	cmpwi 7,26,2
	li 11,1
	lwz 0,28(9)
	add 0,0,28
	stw 0,28(9)
	bne 7,.L85
	lwz 0,16(27)
	add 0,0,28
	stw 0,16(27)
	b .L85
.L78:
	cmpwi 7,26,0
	beq 7,.L88
	addi 9,26,-1
	slwi 9,9,3
	add 9,9,27
	lwz 11,28(9)
	lwz 0,24(9)
	add 0,0,11
	b .L90
.L88:
	lwz 9,0(27)
	addi 0,9,40
.L90:
	subf 3,28,25
	li 11,0
	cmplw 7,0,3
	bgt 7,.L85
	cmplw 7,29,25
	ble 7,.L93
	mr 4,25
	subf 5,25,29
	bl memmove+32768@plt
	cmpwi 7,26,1
	bne 7,.L93
	lwz 9,260(27)
	neg 6,28
	li 7,1
	cmplwi 7,9,1
	bge+ 7,.L104
	li 9,1
	b .L104
.L97:
	lwz 11,252(27)
	lwzx 0,11,8
	cmplw 7,0,29
	add 10,0,6
	bge 7,.L104
	stwx 10,11,8
.L104:
	addic. 9,9,-1
	slwi 8,7,2
	addi 7,7,1
	bne 0,.L97
.L93:
	lwz 0,0(24)
	slwi 9,26,3
	add 9,9,27
	li 11,1
	subf 0,28,0
	stw 0,0(24)
	lwz 0,28(9)
	add 0,0,28
	stw 0,28(9)
	lwz 0,24(9)
	subf 0,28,0
	stw 0,24(9)
.L85:
	lwz 0,52(1)
	lwz 12,12(1)
	mr 3,11
	lmw 24,16(1)
	addi 1,1,48
	mtlr 0
	mtcrf 8,12
	blr
	.size	ft_shuffle,.-ft_shuffle
	.align 2
	.globl ft_get_prop
	.type	ft_get_prop, @function
ft_get_prop:
	stwu 1,-48(1)
	mflr 0
	bcl 20,31,.LCF9
.LCF9:
	stw 0,52(1)
	stmw 27,28(1)
	mflr 30
	mr 27,6
	mr 29,5
	addis 30,30,.LCTOC1-.LCF9@ha
	mr 28,3
	addi 30,30,.LCTOC1-.LCF9@l
	mr 31,7
	bl ft_node_ph2node@local
	mr 5,29
	addi 6,1,8
	mr. 4,3
	mr 3,28
	beq 0,.L106
	bl __ft_get_prop@local
	mr 5,31
	mr. 4,3
	mr 3,27
	beq 0,.L106
	lwz 0,8(1)
	cmplw 7,31,0
	ble 7,.L109
	mr 5,0
.L109:
	bl memcpy+32768@plt
	lwz 3,8(1)
	b .L110
.L106:
	li 3,-1
.L110:
	lwz 0,52(1)
	lmw 27,28(1)
	addi 1,1,48
	mtlr 0
	blr
	.size	ft_get_prop,.-ft_get_prop
	.align 2
	.type	ft_put_bin, @function
ft_put_bin:
	stwu 1,-32(1)
	mflr 0
	bcl 20,31,.LCF10
.LCF10:
	stw 0,36(1)
	addi 0,5,3
	stmw 29,20(1)
	mflr 30
	rlwinm 29,0,0,0,29
	mr 31,3
	addis 30,30,.LCTOC1-.LCF10@ha
	cmplw 7,5,29
	addi 30,30,.LCTOC1-.LCF10@l
	bge 7,.L113
	lwz 9,20(3)
	li 0,0
	add 9,9,29
	stw 0,-4(9)
.L113:
	lwz 3,20(31)
	bl memcpy+32768@plt
	lwz 0,20(31)
	add 0,0,29
	stw 0,20(31)
	lwz 0,36(1)
	lmw 29,20(1)
	addi 1,1,32
	mtlr 0
	blr
	.size	ft_put_bin,.-ft_put_bin
	.align 2
	.type	ft_make_space, @function
ft_make_space:
	stwu 1,-48(1)
	mflr 0
	bcl 20,31,.LCF11
.LCF11:
	stw 0,52(1)
	stmw 22,8(1)
	mflr 30
	mr 31,3
	mr 23,4
	lwz 0,8(3)
	addis 30,30,.LCTOC1-.LCF11@ha
	mr 26,5
	addi 30,30,.LCTOC1-.LCF11@l
	mr 25,6
	cmpwi 7,0,0
	bne 7,.L117
	slwi 0,5,3
	add 24,0,3
	lwz 9,28(3)
	lwz 0,36(3)
	cmpwi 7,6,0
	lwz 8,0(4)
	lwz 10,44(3)
	addi 9,9,1064
	lwz 11,24(24)
	add 9,9,0
	add 9,9,10
	subf 22,11,8
	ble 7,.L119
	add 9,9,6
.L119:
	lwz 11,12(31)
	cmpwi 7,11,0
	beq 7,.L121
	addi 0,9,7
	li 3,0
	rlwinm 27,0,0,0,28
	mr 4,27
	mtctr 11
	bctrl
	mr. 28,3
	beq 0,.L121
	lwz 4,0(31)
	li 5,36
	addi 29,28,40
	bl memcpy+32768@plt
	stw 28,0(31)
	stw 27,4(31)
	mr 3,29
	lwz 4,24(31)
	lwz 5,28(31)
	bl memcpy+32768@plt
	stw 29,24(31)
	lwz 0,28(31)
	lwz 4,32(31)
	lwz 5,36(31)
	add 29,29,0
	mr 3,29
	bl memcpy+32768@plt
	lwz 4,32(31)
	mr 3,31
	subf 5,4,29
	bl ft_node_update_after@local
	lwz 0,32(31)
	lwz 9,20(31)
	stw 29,32(31)
	lwz 5,44(31)
	subf 0,0,29
	add 29,28,27
	lwz 4,40(31)
	add 9,9,0
	subf 29,5,29
	stw 9,20(31)
	mr 3,29
	bl memcpy+32768@plt
	lwz 0,16(31)
	lwz 9,40(31)
	stw 29,40(31)
	subf 0,9,0
	add 29,29,0
	li 0,1
	stw 29,16(31)
	stw 0,8(31)
	lwz 0,24(24)
	add 0,0,22
	stw 0,0(23)
.L117:
	mr 3,31
	mr 4,23
	mr 5,26
	mr 6,25
	bl ft_shuffle@local
	cmpwi 7,3,0
	bne 7,.L124
	lwz 24,44(31)
	lwz 4,40(31)
	lwz 11,0(31)
	lwz 0,4(31)
	add 9,4,24
	add 11,11,0
	cmplw 7,9,11
	bge 7,.L126
	subf 29,24,11
	lwz 0,16(31)
	mr 5,24
	subf 9,4,29
	mr 3,29
	add 0,0,9
	stw 0,16(31)
	bl memmove+32768@plt
	cmpwi 7,26,0
	stw 29,40(31)
	beq 7,.L126
	mr 3,31
	mr 4,23
	mr 5,26
	mr 6,25
	bl ft_shuffle@local
	cmpwi 7,3,0
	bne 7,.L124
.L126:
	slwi 0,26,3
	add 29,0,31
	mr 28,26
	li 27,0
	b .L129
.L130:
	bl next_start@local
	lwz 0,24(29)
	lwz 9,28(29)
	addi 29,29,8
	add 0,0,9
	subf 3,0,3
	add 27,27,3
.L129:
	cmplwi 7,28,1
	mr 3,31
	mr 4,26
	addi 28,28,1
	ble 7,.L130
	cmplw 7,27,25
	bge 7,.L132
	lwz 11,12(31)
	cmpwi 7,11,0
	beq 7,.L121
	lwz 9,4(31)
	lwz 3,0(31)
	subf 0,27,25
	mtctr 11
	addi 9,9,1031
	add 0,0,9
	rlwinm 27,0,0,0,28
	mr 4,27
	bctrl
	mr. 28,3
	beq 0,.L121
	lwz 0,0(31)
	stw 27,4(31)
	subf. 29,0,28
	beq 0,.L136
	stw 28,0(31)
	lwz 4,32(31)
	mr 3,31
	mr 5,29
	bl ft_node_update_after@local
	lwz 0,24(31)
	lwz 9,32(31)
	lwz 11,40(31)
	add 0,0,29
	add 9,9,29
	add 11,11,29
	stw 0,24(31)
	stw 9,32(31)
	stw 11,40(31)
	lwz 0,0(23)
	add 0,0,29
	stw 0,0(23)
	lwz 0,16(31)
	add 0,0,29
	stw 0,16(31)
.L136:
	lwz 4,40(31)
	lwz 0,16(31)
	add 29,28,27
	mr 5,24
	subf 29,24,29
	subf 9,4,29
	mr 3,29
	add 0,0,9
	stw 0,16(31)
	bl memmove+32768@plt
	stw 29,40(31)
	mr 3,31
	mr 4,23
	mr 5,26
	mr 6,25
	bl ft_shuffle@local
	cmpwi 7,3,0
	bne 7,.L124
.L132:
	cmpwi 7,26,0
	bne 7,.L121
	lwz 9,28(31)
	lwz 0,24(31)
	lwz 5,36(31)
	add 0,0,9
	lwz 9,40(31)
	add 29,0,25
	add 0,29,5
	cmplw 7,0,9
	bge 7,.L121
	lwz 4,32(31)
	mr 3,29
	bl memmove+32768@plt
	lwz 4,32(31)
	mr 3,31
	mr 5,25
	bl ft_node_update_after@local
	stw 29,32(31)
	mr 3,31
	mr 4,23
	mr 6,25
	li 5,0
	bl ft_shuffle@local
	addic 9,3,-1
	subfe 0,9,3
	mr 3,0
	b .L140
.L121:
	li 3,0
	b .L140
.L124:
	li 3,1
.L140:
	lwz 0,52(1)
	lmw 22,8(1)
	addi 1,1,48
	mtlr 0
	blr
	.size	ft_make_space,.-ft_make_space
	.align 2
	.globl ft_del_prop
	.type	ft_del_prop, @function
ft_del_prop:
	stwu 1,-48(1)
	mflr 0
	bcl 20,31,.LCF12
.LCF12:
	stw 0,52(1)
	stmw 27,28(1)
	mflr 30
	mr 27,5
	mr 29,3
	addis 30,30,.LCTOC1-.LCF12@ha
	addi 30,30,.LCTOC1-.LCF12@l
	bl ft_node_ph2node@local
	cmpwi 0,3,0
	beq 0,.L143
	mr 31,3
	b .L145
.L146:
	lwz 0,8(1)
	cmplwi 7,0,1
	cmpwi 1,0,3
	cmplwi 6,0,2
	blt 7,.L147
	mr 4,27
	ble 6,.L143
	bne 1,.L147
	lwz 3,12(1)
	bl strcmp+32768@plt
	cmpwi 7,3,0
	bne 7,.L147
	stw 31,20(29)
	lwz 6,20(1)
	mr 3,29
	addi 4,29,20
	li 5,1
	addi 6,6,3
	rlwinm 6,6,0,0,29
	addi 6,6,-12
	bl ft_make_space@local
	li 0,0
	cmpwi 7,3,0
	bne 7,.L151
	b .L143
.L147:
	mr 31,28
.L145:
	mr 4,31
	addi 5,1,8
	mr 3,29
	bl ft_next@local
	mr. 28,3
	bne 0,.L146
.L143:
	li 0,-1
.L151:
	mr 3,0
	lwz 0,52(1)
	lmw 27,28(1)
	addi 1,1,48
	mtlr 0
	blr
	.size	ft_del_prop,.-ft_del_prop
	.align 2
	.globl ft_add_rsvmap
	.type	ft_add_rsvmap, @function
ft_add_rsvmap:
	stwu 1,-48(1)
	mflr 0
	stw 0,52(1)
	stmw 26,24(1)
	addi 4,1,8
	mr 27,6
	mr 26,5
	li 6,16
	lwz 0,28(3)
	lwz 9,24(3)
	li 5,0
	mr 29,8
	mr 28,7
	add 9,9,0
	addi 9,9,-16
	stw 9,8(1)
	bl ft_make_space@local
	li 0,-1
	cmpwi 7,3,0
	beq 7,.L156
	lwz 9,8(1)
	li 0,0
	stw 28,8(9)
	stw 29,12(9)
	stw 26,0(9)
	stw 27,4(9)
.L156:
	lmw 26,24(1)
	mr 3,0
	lwz 0,52(1)
	addi 1,1,48
	mtlr 0
	blr
	.size	ft_add_rsvmap,.-ft_add_rsvmap
	.align 2
	.globl ft_find_descendent
	.type	ft_find_descendent, @function
ft_find_descendent:
	stwu 1,-272(1)
	mflr 0
	bcl 20,31,.LCF14
.LCF14:
	stw 0,276(1)
	stmw 24,240(1)
	mflr 30
	mr 25,3
	mr 24,4
	addis 30,30,.LCTOC1-.LCF14@ha
	mr 29,5
	addi 30,30,.LCTOC1-.LCF14@l
	li 27,0
	li 28,-1
	li 26,0
	b .L195
.L160:
	lwz 0,8(1)
	cmpwi 7,0,1
	beq 7,.L161
	cmpwi 7,0,2
	bne 7,.L195
	b .L162
.L161:
	addi 28,28,1
	cmpw 7,28,26
	bne 7,.L195
	addi 26,28,1
	slwi 9,28,2
	slwi 11,26,2
	add 9,9,25
	add 11,11,25
	lwz 0,16(1)
	cmpwi 7,28,0
	stw 0,48(9)
	li 0,0
	stw 0,48(11)
	beq 7,.L164
	lwz 3,12(1)
	mr 4,29
	mr 5,27
	bl strncmp+32768@plt
	cmpwi 7,3,0
	bne 7,.L166
	lwz 9,12(1)
	lbzx 0,27,9
	cmpwi 7,0,47
	beq 7,.L164
	cmpwi 7,0,0
	beq 7,.L164
	cmpwi 7,0,64
	bne 7,.L166
.L164:
	add 31,29,27
	b .L170
.L171:
	addi 31,31,1
.L170:
	lbz 0,0(31)
	cmpwi 7,0,47
	beq 7,.L171
	cmpwi 7,0,0
	bne 7,.L173
	lwz 3,16(1)
	b .L175
.L173:
	mr 3,31
	li 4,47
	bl strchr+32768@plt
	cmpwi 0,3,0
	beq 0,.L176
	subf 27,31,3
	b .L178
.L176:
	mr 3,31
	bl strlen+32768@plt
	mr 27,3
.L178:
	slwi 0,28,2
	add 9,1,0
	stw 29,24(9)
	mr 29,31
	b .L195
.L162:
	cmpwi 7,28,0
	beq 7,.L179
	cmpw 7,26,28
	ble 7,.L181
	addi 26,26,-1
	mr 0,29
	slwi 9,26,2
	add 9,9,31
	lwz 29,16(9)
	subf 0,29,0
	addic. 27,0,-1
	mr 9,0
	add 11,29,0
	bge+ 0,.L183
	li 9,1
	b .L183
.L184:
	addi 27,27,-1
.L183:
	addic. 9,9,-1
	beq 0,.L181
	lbz 0,-2(11)
	addi 11,11,-1
	cmpwi 7,0,47
	beq 7,.L184
.L181:
	addi 28,28,-1
	b .L195
.L166:
	mr 26,28
.L195:
	addi 31,1,8
	mr 4,24
	mr 3,25
	mr 5,31
	bl ft_next@local
	mr. 24,3
	bne 0,.L160
.L179:
	li 3,0
.L175:
	lwz 0,276(1)
	lmw 24,240(1)
	addi 1,1,272
	mtlr 0
	blr
	.size	ft_find_descendent,.-ft_find_descendent
	.align 2
	.globl ft_find_device
	.type	ft_find_device, @function
ft_find_device:
	stwu 1,-32(1)
	cmpwi 7,4,0
	mflr 0
	bcl 20,31,.LCF15
.LCF15:
	stw 0,36(1)
	stmw 29,20(1)
	mflr 30
	mr 29,5
	mr 31,3
	addis 30,30,.LCTOC1-.LCF15@ha
	addi 30,30,.LCTOC1-.LCF15@l
	beq 7,.L197
	bl ft_node_ph2node@local
	cmpwi 0,3,0
	bne 0,.L199
	b .L204
.L197:
	lwz 3,32(3)
.L199:
	mr 4,3
	mr 5,29
	mr 3,31
	bl ft_find_descendent+32768@plt
	lwz 0,36(1)
	mtlr 0
	mr 4,3
	mr 3,31
	lmw 29,20(1)
	addi 1,1,32
	b ft_get_phandle@local
.L204:
	lwz 0,36(1)
	lmw 29,20(1)
	li 3,0
	addi 1,1,32
	mtlr 0
	blr
	.size	ft_find_device,.-ft_find_device
	.align 2
	.globl ft_open
	.type	ft_open, @function
ft_open:
	stwu 1,-64(1)
	mflr 0
	bcl 20,31,.LCF16
.LCF16:
	stw 0,68(1)
	stmw 25,36(1)
	mflr 30
	mr 28,4
	mr 25,5
	lwz 0,20(4)
	addis 30,30,.LCTOC1-.LCF16@ha
	mr 26,7
	addi 30,30,.LCTOC1-.LCF16@l
	mr 31,3
	cmplwi 7,0,15
	ble+ 7,.L206
	addi 27,6,1
	li 4,0
	li 5,264
	slwi 29,27,2
	bl memset+32768@plt
	li 3,0
	mr 4,29
	mtctr 26
	bctrl
	cmpwi 7,3,0
	stw 3,252(31)
	beq 7,.L206
	mr 5,29
	li 4,0
	bl memset+32768@plt
	li 0,1
	stw 27,256(31)
	stw 25,4(31)
	stw 0,260(31)
	stw 28,0(31)
	stw 26,12(31)
	lwz 0,16(28)
	add 8,28,0
	mr 10,8
	stw 8,24(31)
	b .L209
.L210:
	addi 10,10,16
.L209:
	lwz 0,0(10)
	lwz 9,4(10)
	or. 11,0,9
	bne 0,.L210
	lwz 0,8(10)
	lwz 9,12(10)
	or. 11,0,9
	bne 0,.L210
	lwz 0,8(28)
	lwz 11,36(31)
	subf 9,8,10
	addi 9,9,16
	add 29,28,0
	cmpwi 7,11,0
	stw 9,28(31)
	stw 29,32(31)
	bne 7,.L219
	nor 0,29,29
	stw 0,36(31)
.L219:
	mr 4,29
	mr 3,31
	addi 5,1,8
	bl ft_next@local
	cmpwi 0,3,0
	beq 0,.L215
	mr 29,3
	b .L219
.L215:
	lwz 11,32(31)
	lwz 0,12(28)
	li 3,0
	lwz 10,32(28)
	stw 11,20(31)
	subf 9,11,29
	add 0,28,0
	stw 10,44(31)
	addi 9,9,4
	stw 0,16(31)
	stw 0,40(31)
	stw 9,36(31)
	b .L217
.L206:
	li 3,-1
.L217:
	lwz 0,68(1)
	lmw 25,36(1)
	addi 1,1,64
	mtlr 0
	blr
	.size	ft_open,.-ft_open
	.align 2
	.globl ft_begin
	.type	ft_begin, @function
ft_begin:
	stwu 1,-32(1)
	mflr 0
	bcl 20,31,.LCF17
.LCF17:
	stw 0,36(1)
	stmw 26,8(1)
	mflr 30
	mr 29,3
	mr 27,4
	addis 30,30,.LCTOC1-.LCF17@ha
	mr 26,5
	addi 30,30,.LCTOC1-.LCF17@l
	li 4,0
	li 5,264
	mr 28,6
	bl memset+32768@plt
	li 0,1
	stw 26,4(29)
	stw 27,0(29)
	mr 3,27
	stw 0,8(29)
	stw 28,12(29)
	li 4,0
	li 5,36
	add 26,27,26
	bl memset+32768@plt
	lis 0,0xd00d
	li 9,16
	stw 27,16(29)
	stw 26,40(29)
	ori 0,0,65261
	addi 8,27,40
	stw 9,28(29)
	stw 9,20(27)
	stw 0,0(27)
	stw 9,24(27)
	li 0,4
	addi 11,27,56
	stw 0,36(29)
	stw 11,32(29)
	li 0,0
	li 9,0
	stw 0,44(29)
	stw 8,24(29)
	li 0,9
	li 10,0
	stw 0,56(27)
	stw 9,40(27)
	lwz 0,36(1)
	stw 10,44(27)
	lmw 26,8(1)
	addi 1,1,32
	stw 9,8(8)
	mtlr 0
	stw 10,12(8)
	blr
	.size	ft_begin,.-ft_begin
	.align 2
	.globl ft_begin_tree
	.type	ft_begin_tree, @function
ft_begin_tree:
	stwu 1,-16(1)
	lwz 0,32(3)
	stw 0,20(3)
	addi 1,1,16
	blr
	.size	ft_begin_tree,.-ft_begin_tree
	.align 2
	.globl ft_get_path
	.type	ft_get_path, @function
ft_get_path:
	stwu 1,-272(1)
	mflr 0
	bcl 20,31,.LCF19
.LCF19:
	stw 0,276(1)
	stmw 24,240(1)
	mflr 30
	mr 25,5
	mr 26,6
	addis 30,30,.LCTOC1-.LCF19@ha
	mr 27,3
	addi 30,30,.LCTOC1-.LCF19@l
	bl ft_node_ph2node@local
	mr. 24,3
	beq 0,.L225
	lwz 31,32(27)
	li 28,0
	b .L227
.L228:
	lwz 0,8(1)
	cmpwi 7,0,1
	cmpwi 6,0,2
	beq 7,.L230
	bne 6,.L229
	b .L231
.L230:
	lwz 0,12(1)
	addi 28,28,1
	stw 0,16(9)
	bne 1,.L229
	b .L232
.L231:
	addic. 28,28,-1
	beq 0,.L225
.L229:
	mr 31,3
.L227:
	addi 29,1,8
	mr 4,31
	mr 3,27
	mr 5,29
	bl ft_next@local
	slwi 0,28,2
	add 9,0,29
	cmpw 1,31,24
	cmpwi 0,3,0
	bne 0,.L228
.L232:
	mr 9,25
	li 27,1
	b .L233
.L234:
	ble 6,.L225
	li 0,47
	stb 0,0(9)
	addi 9,1,24
	lwzx 4,9,11
	bl strncpy+32768@plt
	add 9,31,29
	mr 3,31
	lbz 0,-1(9)
	cmpwi 7,0,0
	bne 7,.L225
	bl strlen+32768@plt
	add 9,31,3
	subf 26,3,29
.L233:
	addi 31,9,1
	addi 29,26,-1
	cmpw 7,27,28
	slwi 11,27,2
	mr 5,29
	mr 3,31
	addi 27,27,1
	cmpwi 6,26,1
	blt 7,.L234
	b .L237
.L225:
	li 25,0
.L237:
	lwz 0,276(1)
	mr 3,25
	lmw 24,240(1)
	addi 1,1,272
	mtlr 0
	blr
	.size	ft_get_path,.-ft_get_path
	.align 2
	.globl __ft_find_node_by_prop_value
	.type	__ft_find_node_by_prop_value, @function
__ft_find_node_by_prop_value:
	stwu 1,-80(1)
	mflr 0
	bcl 20,31,.LCF20
.LCF20:
	stw 0,84(1)
	stmw 22,40(1)
	mflr 30
	cntlzw 29,4
	addis 30,30,.LCTOC1-.LCF20@ha
	lwz 31,32(3)
	mr 28,3
	addi 30,30,.LCTOC1-.LCF20@l
	mr 25,4
	mr 22,5
	mr 23,6
	mr 26,7
	srwi 29,29,5
	li 27,-1
	b .L240
.L241:
	lwz 0,12(1)
	cmpwi 7,0,1
	beq 7,.L243
	cmpwi 7,0,2
	bne 7,.L242
	b .L244
.L243:
	cmpw 7,25,31
	addi 27,27,1
	bne 7,.L245
	li 29,1
	b .L242
.L245:
	cmpwi 7,29,0
	beq 7,.L242
	cmpwi 7,27,0
	ble 7,.L242
	mr 3,28
	mr 4,31
	mr 5,22
	addi 6,1,8
	bl __ft_get_prop@local
	cmpwi 7,3,0
	beq 7,.L242
	lwz 0,8(1)
	cmpw 7,0,26
	bne 7,.L242
	mr 4,23
	mr 5,26
	bl memcmp+32768@plt
	cmpwi 7,3,0
	bne 7,.L242
	mr 3,31
	b .L252
.L244:
	cmpwi 7,27,0
	addi 27,27,-1
	beq 7,.L253
.L242:
	mr 31,24
.L240:
	mr 3,28
	mr 4,31
	addi 5,1,12
	bl ft_next@local
	mr. 24,3
	bne 0,.L241
.L253:
	li 3,0
.L252:
	lwz 0,84(1)
	lmw 22,40(1)
	addi 1,1,80
	mtlr 0
	blr
	.size	__ft_find_node_by_prop_value,.-__ft_find_node_by_prop_value
	.align 2
	.globl ft_find_node_by_prop_value
	.type	ft_find_node_by_prop_value, @function
ft_find_node_by_prop_value:
	stwu 1,-32(1)
	cmpwi 7,4,0
	mflr 0
	bcl 20,31,.LCF21
.LCF21:
	stw 0,36(1)
	li 0,0
	stmw 27,12(1)
	mflr 30
	mr 29,5
	mr 28,6
	addis 30,30,.LCTOC1-.LCF21@ha
	mr 27,7
	addi 30,30,.LCTOC1-.LCF21@l
	mr 31,3
	beq 7,.L258
	bl ft_node_ph2node@local
	mr. 0,3
	beq 0,.L262
.L258:
	mr 4,0
	mr 5,29
	mr 6,28
	mr 7,27
	mr 3,31
	bl __ft_find_node_by_prop_value+32768@plt
	lwz 0,36(1)
	mtlr 0
	mr 4,3
	mr 3,31
	lmw 27,12(1)
	addi 1,1,32
	b ft_get_phandle@local
.L262:
	lwz 0,36(1)
	lmw 27,12(1)
	addi 1,1,32
	mtlr 0
	blr
	.size	ft_find_node_by_prop_value,.-ft_find_node_by_prop_value
	.align 2
	.globl __ft_get_parent
	.type	__ft_get_parent, @function
__ft_get_parent:
	stwu 1,-48(1)
	mr 9,3
	mflr 0
	li 11,0
	stw 0,52(1)
	stmw 28,32(1)
	mr 31,3
	mr 28,4
	addi 3,3,44
	b .L264
.L265:
	bne 6,.L266
	cmpwi 7,11,0
	ble 7,.L268
	lwz 3,0(3)
	b .L270
.L266:
	addi 11,11,1
	addi 3,3,4
.L264:
	lwz 0,48(9)
	addi 9,9,4
	cmpwi 7,0,0
	cmpw 6,0,28
	bne 7,.L265
	lwz 3,32(31)
	li 29,0
	b .L285
.L273:
	lwz 0,8(1)
	cmpwi 7,0,1
	cmpwi 6,0,2
	beq 7,.L274
	bne 6,.L285
	b .L275
.L274:
	lwz 0,16(1)
	stw 0,48(9)
	lwz 0,16(1)
	cmpw 7,28,0
	bne 7,.L276
	li 0,0
	cmpwi 7,29,0
	stw 0,52(9)
	ble 7,.L268
	lwz 3,44(9)
	b .L270
.L276:
	addi 29,29,1
	b .L285
.L275:
	addi 29,29,-1
.L285:
	mr 4,3
	addi 5,1,8
	mr 3,31
	bl ft_next@local
	slwi 0,29,2
	add 9,0,31
	cmpwi 0,3,0
	bne 0,.L273
.L268:
	li 3,0
.L270:
	lwz 0,52(1)
	lmw 28,32(1)
	addi 1,1,48
	mtlr 0
	blr
	.size	__ft_get_parent,.-__ft_get_parent
	.align 2
	.globl ft_get_parent
	.type	ft_get_parent, @function
ft_get_parent:
	stwu 1,-16(1)
	mflr 0
	bcl 20,31,.LCF23
.LCF23:
	stw 0,20(1)
	stmw 30,8(1)
	mflr 30
	mr 31,3
	addis 30,30,.LCTOC1-.LCF23@ha
	addi 30,30,.LCTOC1-.LCF23@l
	bl ft_node_ph2node@local
	mr. 4,3
	beq 0,.L291
	mr 3,31
	bl __ft_get_parent+32768@plt
	lwz 0,20(1)
	mtlr 0
	mr 4,3
	mr 3,31
	lmw 30,8(1)
	addi 1,1,16
	b ft_get_phandle@local
.L291:
	lwz 0,20(1)
	lmw 30,8(1)
	addi 1,1,16
	mtlr 0
	blr
	.size	ft_get_parent,.-ft_get_parent
	.align 2
	.globl ft_prop
	.type	ft_prop, @function
ft_prop:
	stwu 1,-64(1)
	mflr 0
	bcl 20,31,.LCF24
.LCF24:
	stw 0,68(1)
	stmw 25,36(1)
	mflr 30
	mr 29,3
	mr 28,4
	lwz 31,40(3)
	lwz 0,44(3)
	addis 30,30,.LCTOC1-.LCF24@ha
	mr 25,5
	addi 30,30,.LCTOC1-.LCF24@l
	mr 26,6
	add 27,31,0
	b .L293
.L294:
	bl strcmp+32768@plt
	cmpwi 7,3,0
	mr 3,31
	bne 7,.L295
	lwz 0,16(29)
	lis 9,0x7fff
	ori 9,9,65535
	subf 31,0,31
	cmpw 7,31,9
	bne 7,.L297
	b .L298
.L295:
	bl strlen+32768@plt
	add 3,31,3
	addi 31,3,1
.L293:
	cmplw 7,31,27
	mr 3,31
	mr 4,28
	blt 7,.L294
.L298:
	lwz 0,40(29)
	mr 3,28
	stw 0,8(1)
	bl strlen+32768@plt
	addi 4,1,8
	li 5,2
	addi 6,3,1
	mr 3,29
	bl ft_make_space@local
	cmpwi 7,3,0
	beq 7,.L299
	lwz 3,8(1)
	mr 4,28
	bl strcpy+32768@plt
	lwz 11,16(29)
	lwz 9,8(1)
	lis 0,0x7fff
	ori 0,0,65535
	subf 31,11,9
	cmpw 7,31,0
	beq 7,.L299
.L297:
	addi 6,26,3
	mr 3,29
	rlwinm 6,6,0,0,29
	addi 4,29,20
	addi 6,6,12
	li 5,1
	bl ft_make_space@local
	cmpwi 7,3,0
	beq 7,.L299
	lwz 9,20(29)
	li 0,3
	mr 3,29
	mr 4,25
	mr 5,26
	addi 11,9,4
	addi 10,11,8
	stw 0,0(9)
	stw 26,4(9)
	stw 10,20(29)
	stw 31,4(11)
	bl ft_put_bin@local
	li 3,0
	b .L302
.L299:
	li 3,-1
.L302:
	lwz 0,68(1)
	lmw 25,36(1)
	addi 1,1,64
	mtlr 0
	blr
	.size	ft_prop,.-ft_prop
	.align 2
	.globl ft_set_prop
	.type	ft_set_prop, @function
ft_set_prop:
	stwu 1,-64(1)
	mflr 0
	bcl 20,31,.LCF25
.LCF25:
	stw 0,68(1)
	stmw 25,36(1)
	mflr 30
	mr 26,5
	mr 25,6
	addis 30,30,.LCTOC1-.LCF25@ha
	mr 28,7
	addi 30,30,.LCTOC1-.LCF25@l
	mr 31,3
	bl ft_node_ph2node@local
	cmpwi 0,3,0
	beq 0,.L305
	mr 4,3
	addi 5,1,8
	mr 3,31
	bl ft_next@local
	lwz 0,8(1)
	cmpwi 7,0,1
	mr 29,3
	beq- 7,.L308
	b .L305
.L309:
	lwz 0,8(1)
	cmplwi 7,0,1
	cmpwi 1,0,3
	cmplwi 6,0,2
	blt 7,.L310
	ble 6,.L311
	bne 1,.L310
	b .L312
.L311:
	stw 29,20(31)
	mr 3,31
	mr 4,26
	mr 5,25
	mr 6,28
	bl ft_prop+32768@plt
	b .L313
.L312:
	lwz 3,12(1)
	mr 4,26
	bl strcmp+32768@plt
	cmpwi 7,3,0
	bne 7,.L310
	lwz 9,20(1)
	addi 0,28,3
	rlwinm 0,0,0,0,29
	addi 9,9,3
	rlwinm 9,9,0,0,29
	subf. 6,9,0
	lwz 0,16(1)
	stw 0,20(31)
	beq 0,.L315
	mr 3,31
	addi 4,31,20
	li 5,1
	bl ft_make_space@local
	cmpwi 7,3,0
	beq 7,.L305
.L315:
	lwz 9,20(31)
	mr 3,31
	mr 4,25
	mr 5,28
	stw 28,-8(9)
	bl ft_put_bin@local
	li 3,0
	b .L313
.L310:
	mr 29,27
.L308:
	mr 4,29
	mr 3,31
	addi 5,1,8
	bl ft_next@local
	mr. 27,3
	bne 0,.L309
.L305:
	li 3,-1
.L313:
	lwz 0,68(1)
	lmw 25,36(1)
	addi 1,1,64
	mtlr 0
	blr
	.size	ft_set_prop,.-ft_set_prop
	.align 2
	.globl ft_prop_int
	.type	ft_prop_int, @function
ft_prop_int:
	stwu 1,-32(1)
	li 6,4
	mflr 0
	bcl 20,31,.LCF26
.LCF26:
	stw 5,8(1)
	stw 0,36(1)
	stmw 30,24(1)
	mflr 30
	addi 5,1,8
	addis 30,30,.LCTOC1-.LCF26@ha
	addi 30,30,.LCTOC1-.LCF26@l
	bl ft_prop+32768@plt
	lwz 0,36(1)
	lmw 30,24(1)
	addi 1,1,32
	mtlr 0
	blr
	.size	ft_prop_int,.-ft_prop_int
	.align 2
	.globl ft_prop_str
	.type	ft_prop_str, @function
ft_prop_str:
	stwu 1,-32(1)
	mflr 0
	bcl 20,31,.LCF27
.LCF27:
	stw 0,36(1)
	stmw 27,12(1)
	mflr 30
	mr 27,3
	mr 3,5
	addis 30,30,.LCTOC1-.LCF27@ha
	mr 28,5
	addi 30,30,.LCTOC1-.LCF27@l
	mr 29,4
	bl strlen+32768@plt
	mr 4,29
	mr 5,28
	addi 6,3,1
	mr 3,27
	bl ft_prop+32768@plt
	lwz 0,36(1)
	lmw 27,12(1)
	addi 1,1,32
	mtlr 0
	blr
	.size	ft_prop_str,.-ft_prop_str
	.align 2
	.globl ft_begin_node
	.type	ft_begin_node, @function
ft_begin_node:
	stwu 1,-32(1)
	mflr 0
	bcl 20,31,.LCF28
.LCF28:
	stw 0,36(1)
	stmw 28,16(1)
	mflr 30
	mr 31,3
	mr 3,4
	addis 30,30,.LCTOC1-.LCF28@ha
	mr 28,4
	addi 30,30,.LCTOC1-.LCF28@l
	li 29,0
	bl strlen+32768@plt
	addi 4,31,20
	li 5,1
	addi 6,3,4
	mr 3,31
	rlwinm 6,6,0,0,29
	addi 6,6,8
	bl ft_make_space@local
	cmpwi 7,3,0
	mr 3,28
	beq 7,.L325
	lwz 29,20(31)
	addi 0,29,4
	stw 0,20(31)
	li 0,1
	stw 0,0(29)
	bl strlen+32768@plt
	mr 4,28
	addi 5,3,1
	mr 3,31
	bl ft_put_bin@local
.L325:
	lwz 0,36(1)
	mr 3,29
	lmw 28,16(1)
	addi 1,1,32
	mtlr 0
	blr
	.size	ft_begin_node,.-ft_begin_node
	.align 2
	.globl ft_create_node
	.type	ft_create_node, @function
ft_create_node:
	stwu 1,-48(1)
	cmpwi 7,4,0
	mflr 0
	bcl 20,31,.LCF29
.LCF29:
	stw 0,52(1)
	stmw 26,24(1)
	mflr 30
	mr 26,5
	mr 31,3
	addis 30,30,.LCTOC1-.LCF29@ha
	addi 30,30,.LCTOC1-.LCF29@l
	beq 7,.L328
	bl ft_node_ph2node@local
	mr. 29,3
	beq 0,.L330
.L331:
	li 28,0
	b .L332
.L328:
	lwz 29,32(3)
	b .L331
.L341:
	stw 29,20(31)
	mr 4,26
	mr 3,31
	bl ft_begin_node+32768@plt
	mr 29,3
	mr 3,31
	bl ft_end_node+32768@plt
	mr 3,31
	mr 4,29
	bl ft_get_phandle@local
	b .L339
.L333:
	lwz 0,8(1)
	cmpwi 7,0,1
	cmpwi 6,0,2
	beq 7,.L335
	bne 6,.L334
	b .L336
.L335:
	mr 4,26
	addi 28,28,1
	bne+ 1,.L334
	lwz 3,12(1)
	bl strcmp+32768@plt
	cmpwi 7,3,0
	bne 7,.L334
	b .L330
.L336:
	addic. 28,28,-1
	ble 0,.L341
.L334:
	mr 29,27
.L332:
	mr 4,29
	addi 5,1,8
	mr 3,31
	bl ft_next@local
	cmpwi 1,28,0
	mr. 27,3
	bne 0,.L333
.L330:
	li 3,0
.L339:
	lwz 0,52(1)
	lmw 26,24(1)
	addi 1,1,48
	mtlr 0
	blr
	.size	ft_create_node,.-ft_create_node
	.align 2
	.globl ft_nop
	.type	ft_nop, @function
ft_nop:
	stwu 1,-16(1)
	addi 4,3,20
	mflr 0
	li 5,1
	li 6,4
	stw 31,8(1)
	stw 0,20(1)
	mr 31,3
	bl ft_make_space@local
	cmpwi 7,3,0
	beq 7,.L345
	lwz 9,20(31)
	addi 0,9,4
	stw 0,20(31)
	li 0,4
	stw 0,0(9)
.L345:
	lwz 0,20(1)
	lwz 31,8(1)
	addi 1,1,16
	mtlr 0
	blr
	.size	ft_nop,.-ft_nop
	.ident	"GCC: (GNU) 4.1.2 20070115 (prerelease) (SUSE Linux)"
	.section	.note.GNU-stack,"",@progbits
