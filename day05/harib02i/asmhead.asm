; haribote-os boot asm
; TAB=4

BOTPAK	EQU	0x00280000
DSKCAC	EQU	0x00100000
DSKCAC0	EQU	0x00008000

CYLS	EQU	0x0ff0
LEDS	EQU	0x0ff1
VMODE	EQU	0x0ff2
SCRNX	EQU	0x0ff4
SCRNY	EQU	0x0ff6
VRAM	EQU	0x0ff8

	ORG	0xc400

;
	MOV	AL, 0x13
	MOV	AH, 0x00
	INT	0x10		; 设置VGA显卡，320x200x8位彩色
	MOV	BYTE [VMODE], 8
	MOV	WORD [SCRNX], 320
	MOV	WORD [SCRNY], 200
	MOV	DWORD [VRAM], 0x000a0000

;
	MOV	AH, 0x02
	INT	0x16
	MOV	[LEDS], AL

;
	MOV	AL, 0xff
	OUT	0x21, AL
	NOP

	OUT	0xa1, AL	;写两个端口，初始化关闭PIC，也算关了NMIs。

	CLI

	CALL	waitkbdout
	MOV	AL, 0xd1
	OUT	0x64, AL
	CALL	waitkbdout
	MOV	AL, 0xdf
	OUT	0x60, AL
	CALL	waitkbdout	; 键盘控制器的附属端口，用于打开A20地址线

;[INSTRSET "i486p"]，虽然是16位指令，但可以使用32位寄存器之类的。
	LGDT 	[GDTR0]
	MOV	EAX, CR0
	AND	EAX, 0x7fffffff
	OR	EAX, 0x00000001
	MOV	CR0, EAX	; 进入保护模式，保护模式的特性只是在相应的地方起作用，如跳转时的检测，分页的检测等，此时由于没有改变CS的值，CPU使用的基地址没有改变（基地址只在改变CS的时读取选择子然后查找描述符时改变），所以程序继续执行，几乎和刚刚的实地址模式没有区别。	
	JMP	pipelineflush 	
pipelineflush:
	MOV	AX, 1*8		; 段选择子，16位。高13位代表序号，然后TI，RPL
	MOV	DS, AX
	MOV	ES, AX
	MOV	FS, AX
	MOV	GS, AX
	MOV	SS, AX		; 数据段，堆栈段等使用同一个段。已进入保护模式，加载段寄存器时，可见部分和不可见部分都会修改。

; 把0x7c00处即MBR或第一个扇区代码拷贝到DSKCAC=0x00100000处
	MOV	ESI, 0x7c00
	MOV	EDI, DSKCAC
	MOV	ECX, 512/4
	CALL	memcpy

; 把从0x8200即第二个扇区开始处代码拷贝到DSKCAC+0x200处，共18＊2 - 1个扇区，所以前18个柱面现在在DSKCAC=0x00100000处。
	MOV	ESI, DSKCAC0+512
	MOV	EDI, DSKCAC+512
	MOV	ECX, 0
	MOV	CL, BYTE[CYLS]
	IMUL	ECX, 512*18*2/4
	SUB	ECX,512/4
	CALL	memcpy

; 32位代码拷贝到BOTPAK=0x00280000处，按elf格式展开
;	PUSH	BOTPAK		
	PUSH	bootpack
	CALL	initharibsys

;	MOV	EBX, BOTPAK
;	MOV	ECX, [EBX+16]
;  	ADD	ECX, 3
;	SHR	ECX, 2
;	JZ	skip
;	MOV	ESI, [EBX+20]
;	ADD	ESI, EBX
;	MOV	EDI, [EBX+12]
;	CALL	memcpy		
skip:
;	MOV	ESP, [EBX+12]	;
	JMP	DWORD 2*8:0x00001000 ; 改变CS的值，基地址也随之改变，剥离出文件格式信息时失败，还是使用入口偏移，此ELF功能简单，偏移几乎不变，也不需要映射段和重定位。

waitkbdout:
	IN	AL, 0x64
	AND	AL, 0x02
	JNZ	waitkbdout
	RET

memcpy:
	MOV	EAX, [ESI]
	ADD	ESI, 4
	MOV	[EDI], EAX
	ADD	EDI, 4
	SUB	ECX, 1
	JNZ	memcpy
	RET
		
GDT0:
	TIMES 8 DB 0
	DW	0xffff, 0x0000, 0x9200, 0x00cf
	DW	0xffff, 0x0000, 0x9a28, 0x0047

	DW	0
GDTR0:
	DW	8*3-1
	DD	GDT0

initharibsys:
	PUSH 	EBP
	MOV	EBP, ESP 
	MOV	AX, [EBP+0x6] ; [EBP+0x6] elf文件所在的内存地址
	MOVZX	EAX, AX
	MOV	CX, [EAX+0x2c] ; e_phnum;
	MOVZX	ECX, CX
	MOV	EBX, [EAX+0x1c]	; e_phoff
	ADD	EBX, EAX	; EBX->ph
COPYSEC:
	CMP	ECX, 0
	JE	ret
	MOV	ESI, [EBX+0x4]
	ADD	ESI, EAX
	MOV	EDI, [EBX+0x8]
	PUSH	ECX
	MOV	ECX, [EBX+0x10]
	ADD	ECX, 3
	SHR	ECX, 2
	CMP	ECX, 0
	JE	ret0
	PUSH	EAX
	CALL	memcpy
	POP	EAX
	POP	ECX
	SUB	ECX, 1
	MOV	DX, [EAX+0x2a]
	MOVZX	EDX, DX
	ADD	EBX, EDX
	JMP	COPYSEC
ret0:
	POP	ECX
ret:
	MOV	ESI, [EAX+0x18]
	SUB	ESI, 0x280000
;	PUSH	2*8
;	PUSH	SI
;	RETF			; 通过RETF指令修改CS：IP			
;	JMP	2*8:ESI		; JMP far 修改CS：IP，但不支持变量，如ESI
; 	JMP	DWORD 2*8:0x1234	; 没有问题
	MOV 	DWORD [offset], ESI 	; 使用嵌入式，硬编码
	DB 0x66
	DB 0xea
offset:
	DD 0
selector:
	DW 2*8
	RET
	
bootpack:	
