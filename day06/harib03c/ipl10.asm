; hello-os
; TAB=4

	CYLS	EQU	10
	
	ORG	0x7c00	       		; 指明程序的装载地址
	
; 以下的记述用于标准的FAT12格式的软盘
	JMP	entry
	DB	0x90
	
;  
	DB	"HELLOIPL"		; 启动去的名称可以是任意的字符串（8字节）
	DW	512			; 扇区大小（必须512）
	DB	1			; 簇的大小（必须一个扇区）
	DW	1			; FAT的起始位置（一般从第一个扇区开始）
	DB	2			; FAT的个数（必须为2）
	DW	224			; 根目录的大小（一般设成224项）
	DW	2880			; 该磁盘的大小（必须2880扇区）
	DB	0xf0			; 磁盘的种类（必须是0xf0）
	DW	9			; FAT的长度（必须是9扇区）
	DW	18			; 1个磁道有几个扇区（必须是18）
	DW	2			; 磁头数（必须是2）
	DD	0			; 不使用分区，必须是0
	DD	2880			; 重写一次磁盘大小
	DB	0,0,0x29		; 意义不明，固定
	DD	0xffffff		; （可能是）卷表号码
	DB	"HELLO-OS   "		; 磁盘的名称（11字节）
	DB	"FAT12   "		; 磁盘格式名称（8字节）
	times 18 DB 0			; 先空出18个字节

; 程序核心
entry:	
	MOV	AX, 0			; 初始化寄存器
	MOV	SS, AX
	MOV	SP, 0x7c00		; 准备栈
	MOV	DS, AX			; DS默认的地址段寄存器，所以需初始化

; 准备读取软盘A的2-18扇区
	MOV	DL, 0x00		; 驱动器号A
	MOV	CH, 0			; 柱面，从0开始
	MOV	DH, 0			; 磁头，从0开始
	MOV	CL, 2			; 扇区号，1开始
	MOV	AX, 0x0820		; 缓冲区从0x0820:0000开始
	MOV	ES, AX
	MOV	BX, 0			; (ES:BX)缓冲区

readloop:	
	mov	SI, 0
retry:	
	MOV	AH, 0x02		; 读盘
	MOV	AL, 1			; 扇区数
	INT	0x13			; 调用磁盘BIOS
	JNC	next			; 成功，读取下一个扇区
	ADD	SI, 1
	CMP	SI, 5
	JAE	error			; 尝试5次
	MOV	AH, 0x00
	INT	0x13			; 重置驱动器
	JMP	retry
next:
	MOV	AX, ES
	ADD	AX, 0x0020
	MOV	ES, AX
	ADD	CL, 1
	CMP	CL, 18
	JBE	readloop
	MOV	CL, 1			; 下一个磁道，扇区从1开始
	ADD	DH, 1
	CMP	DH, 2
	JB	readloop		; 下一个柱面，磁头从0开始
	MOV	DH, 0
	ADD	CH, 1
	CMP	CH, CYLS
	JB	readloop
	MOV	[0x0ff0], CH		; 
	JMP	0xc400

fin:
	HLT				; 让CPU停止，等待指令
	JMP	fin			; 无限循环

error:
	MOV	SI, msg
putloop:
	MOV	AL, [SI]		; [DS:SI]汇编中可以省略，使用默认
	ADD	SI, 1			; 给SI加1
	CMP	AL, 0
	JE	fin
	MOV	AH, 0x0e		; 显示一个文字
	MOV	BX, 15			; 指定字符颜色
	INT	0x10			; 调用显卡BIOS
	JMP	putloop

msg:
	DB	0x0a, 0x0a		; 换行2次
	DB	"load error!"
	DB	0x0a			; 换行
	DB	0

	times 	0x1fe-($-$$) DB 0
	DB	0x55, 0xaa
