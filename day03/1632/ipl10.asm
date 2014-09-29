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
	MOV	EAX, 0x12345678		; IA-32 real-address mode 可以使用32寄存器
	MOV	AX, 0
	MOV	DWORD	[8000], 0x12345678
	
	
[BITS 32]
	MOV	EAX, 0x12345678
	MOV	AX, 0
	MOV	DWORD	[8000], 0x12345678
	
	times 	0x1fe-($-$$) DB 0
	DB	0x55, 0xaa
