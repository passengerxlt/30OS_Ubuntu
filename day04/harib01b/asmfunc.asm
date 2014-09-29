; asmfunc
; TAC=4

[BITS 32]
GLOBAL _io_hlt,_wirte_mem8

[SECTION .text]

_io_hlt:	; void _io_hlt(void)
	HLT
	RET

_wirte_mem8:	; void write_mem8(int addr, int data);
	MOV	ECX,[ESP+4]	; [ESP+4]中存放的是addr
	MOV	AL,[ESP+8]	; [ESP+8]中存放的是data
	MOV	[ECX],AL	; 16位CX不能指定内存
	RET
	; 32位模式，积极使用32位寄存器，16位寄存器虽然也可使用，但如果使用了的话，不只机器语言的字节会增加，而且执行速度也会变慢。
	; 与C语言联合使用的话，寄存器的使用就会受到限制，能自由使用的只有EAX、ECX、EDX。其他寄存器为只读。因为这些寄存器在C语言编译后生成的机器语言中，用于记忆非常重要的值。
	
