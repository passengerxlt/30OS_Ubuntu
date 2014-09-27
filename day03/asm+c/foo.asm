;

extern choose

[section .data]

num1st		dd	3
num2nd		dd	4

[section .text]

global _start0	;
global myprint	;

_start0:
	PUSH	DWORD [num2nd]
	PUSH	DWORD [num1st]

	CALL	choose
	ADD	ESP, 8

	MOV	EBX, 0
	MOV	EAX, 1
	INT	0x80

myprint:
	MOV	EDX, [ESP + 8]
	MOV	ECX, [ESP + 4]
	MOV	EBX, 1
	MOV	EAX, 4
	INT	0x80
	RET
