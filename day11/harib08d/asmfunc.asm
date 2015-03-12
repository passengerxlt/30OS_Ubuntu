; asmfunc
; TAC=4

[BITS 32]
	GLOBAL _io_hlt,_wirte_mem8, io_cli, io_sti, io_stihlt
	GLOBAL io_in8, io_in16, io_in32
	GLOBAL io_out8, io_out16, io_out32
	GLOBAL memcpy32
	GLOBAL io_load_eflags, io_store_eflags
	GLOBAL load_gdtr, load_idtr
	GLOBAL load_cr0, store_cr0
	GLOBAL io_int21, io_int2c
	GLOBAL asm_inthandler21, asm_inthandler27, asm_inthandler2c
	EXTERN	inthandler21, inthandler27, inthandler2c

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

io_cli:		; void io_cli(void);
	CLI
	RET

io_sti:		; void io_sti(void);
	STI
	RET

io_stihlt:	; void io_stihlt(void);
	STI
	HLT
	RET

io_in8:		; int io_in8(int port);
	MOV	EDX, [ESP+4]	; port
	MOV	EAX, 0
	IN	AL, DX
	RET

io_in16:	; int io_in16(int port);
	MOV	EDX, [ESP+4]	; port
	MOV	EAX, 0
	IN	AX, DX
	RET

io_in32:	; int io_in32(int port);
	MOV	EDX, [ESP+4]	; port
	IN	EAX, DX
	RET

io_out8:	; void io_out8(int port, int data);
	MOV	EDX, [ESP+4]	; port
	MOV	AL, [ESP+8]	; data
	OUT	DX, AL
	RET

io_out16:	; void io_out16(int port, int data);
	MOV	EDX, [ESP+4]	; port
	MOV	AX, [ESP+8]	; data
	OUT	DX, AX
	RET

io_out32:	; void io_out32(int port, int data);
	MOV	EDX, [ESP+4]	; port
	MOV	EAX, [ESP+8]	; data
	OUT	DX, EAX
	RET

io_load_eflags:	; int io_load_eflags(void);
	PUSHFD	; PUSH EFLAGS
	POP	EAX
	RET

io_store_eflags:	; void io_store_eflags(int eflags);
	MOV	EAX, [ESP+4]
	PUSH	EAX
	POPFD	; POP EFLAGS
	RET
load_gdtr:
	MOV	AX, [ESP+4]	;limit
	MOV	[ESP+6], AX
	LGDT	[ESP+6]
	RET
load_idtr:
	MOV	AX, [ESP+4]
	MOV	[ESP+6], AX
	LIDT	[ESP+6]
	RET

asm_inthandler27:
	PUSH 	ES
	PUSH 	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX    		; 参数ESP
	MOV	AX, SS 		; c语言自以为是地认为DS，ES，SS它们都是指同一个段
	MOV	DS, AX
	MOV	ES, AX
	CALL	inthandler27
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

; c语言可以写中断处理，但不能使用IRETD，所以借用汇编实现 
asm_inthandler21:
	PUSH 	ES
	PUSH 	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX    		; 参数ESP
	MOV	AX, SS 		; c语言自以为是地认为DS，ES，SS它们都是指同一个段
	MOV	DS, AX
	MOV	ES, AX
	CALL	inthandler21
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

asm_inthandler2c:
	PUSH 	ES
	PUSH 	DS
	PUSHAD
	MOV	EAX, ESP
	PUSH	EAX    		; 参数ESP
	MOV	AX, SS 		; c语言自以为是地认为DS，ES，SS它们都是指同一个段
	MOV	DS, AX
	MOV	ES, AX
	CALL	inthandler2c
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

io_int21:
	INT	0x21

io_int2c:
	INT	0x2c

load_cr0:	; int load_cr0(void)
	MOV	EAX, CR0
	RET

store_cr0:	; void store_cr0(int)
	MOV	EAX, [ESP+4]
	MOV	CR0, EAX
	RET

memcpy32:	; void memcpy32(char *, char *, int)
	MOV	EDI, [ESP+4]
	MOV	ESI, [ESP+8]
	MOV	ECX, [ESP+12]
	CALL	memcpy
	RET

memcpy:
	MOV	EAX, [ESI]
	ADD	ESI, 4
	MOV	[EDI], EAX
	ADD	EDI, 4
	SUB	ECX, 1
	JNZ	memcpy
	RET
