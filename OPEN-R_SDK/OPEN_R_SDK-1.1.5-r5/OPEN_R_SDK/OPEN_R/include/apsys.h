/*
 *  Copyright 2003 Sony Corporation
 */
#ifndef apsys_h_DEFINED
#define apsys_h_DEFINED

#define GEN_ENTRY(ENTRYNAME, FUNNAME) extern "C" void ENTRYNAME(); \
    asm(\
".align	2\n" \
"	.globl	" #ENTRYNAME "\n" \
"	.ent	" #ENTRYNAME "\n" \
"	.type	" #ENTRYNAME ",@function\n" \
"	.frame  $sp, 0, $31\n" \
"	.cprestore 0	#suppress warning\n" \
#ENTRYNAME ": \n" \
".L" #ENTRYNAME ": \n" \
"	.set	noreorder\n" \
"	ori     $8, $31, 0\n" \
"	bltzal	$0, .L" #ENTRYNAME "\n" \
"	nop\n" \
"	addi	$25, $31, 4\n" \
"	.cpload	$25\n" \
"	.set	reorder\n" \
"	la	$25, " #FUNNAME "\n" \
"	ori	$31, $8, 0\n" \
"	jal	$25\n" \
"	.end	" #ENTRYNAME "\n");


extern "C" int apsysWrite(int fno, const void *buf, int size);
extern "C" int apsysRead(int fno, void *buf, int size);
extern "C" void apsysPut(const char *str);
extern "C" void apsysPutHex(unsigned int);

#endif /* apsys_h_DEFINED */
