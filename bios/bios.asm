cpu 8086
org 0

section .text

	; This must come first, before any other code or includes.
entry:
	db 0x55, 0xaa   ; Marker
	db 0            ; Size / 512
	jmp init        ; Code follows
	db 0

%macro DisplayStringLine 1
section .data
%%str: db %1, 0xd, 0xa, 0
section .text
push si
mov si, %%str
call display_string
pop si
%endmacro
%macro DisplayString 1
section .data
%%str: db %1, 0
section .text
push si
mov si, %%str
call display_string
pop si
%endmacro

REQUEST_DONE: equ 0x00
REQUEST_INIT: equ 0x01
REQUEST_READ: equ 0x02
REQUEST_WRITE: equ 0x03

IO_SEGMENT: equ 0xe000

DATA_BYTES: equ 0x200

CTRL_REQUEST: equ DATA_BYTES
CTRL_CYLINDER_NUMBER: equ DATA_BYTES + 1
CTRL_SECTOR_NUMBER: equ DATA_BYTES + 2
CTRL_HEAD_NUMBER: equ DATA_BYTES + 3
CTRL_DRIVE_NUMBER: equ DATA_BYTES + 4
CTRL_STATUS: equ DATA_BYTES + 5

%undef TRACE

section .text

init:
	push ax
	push bx
	push cx
	push dx
	push ds

	mov ax, cs
	mov ds, ax

	mov ah, 0
	mov al, 3
	int 10h

	DisplayStringLine "Poisk HDD card version 0.1"
	DisplayStringLine "Copyright (c) 2022 Peter Hizalev"

	call install_ipl_diskette
	call install_int13h

	call delay

	pop ds
	pop dx
	pop cx
	pop bx
	pop ax
	retf

int13h:
	pushf
	sti
	cld
	push bp
	push ds

	mov bp, IO_SEGMENT
	mov ds, bp

	cmp ah, 0x3
	jg int13_iret_stc

	mov bp, ax
	xor al, al
	xchg al, ah
	shl ax, 1
	xchg ax, bp
	add bp, int13h_table

	jmp [cs:bp]
	
int13h_table:
	dw int13h_func_reset
	dw int13h_func_status
	dw int13h_func_read
	dw int13h_func_write

int13h_func_reset:
%ifdef TRACE
	DisplayStringLine "reset!"
%endif
	mov byte [CTRL_REQUEST], REQUEST_INIT

int13h_func_reset_1:
	cmp byte [CTRL_REQUEST], REQUEST_DONE
	jne int13h_func_reset_1

	mov byte ah, [CTRL_STATUS]
	call int13h_set_status

	jmp int13_iret

int13h_func_status:
%ifdef TRACE
	DisplayStringLine "status!"
%endif
	call int13h_get_status
	jmp int13_iret_clc

	; AL - number of sectors
	; CH, CL, DH, DL - address of first sector
	; ES:BX - pointer to buffer
int13h_func_read:
%ifdef TRACE
	DisplayStringLine "read!"
%endif
	push cx
	push di
	push si

	mov di, bx

	mov byte [CTRL_CYLINDER_NUMBER], ch
	mov byte [CTRL_SECTOR_NUMBER], cl
	mov byte [CTRL_HEAD_NUMBER], dh
	mov byte [CTRL_DRIVE_NUMBER], dl

int13h_func_read_2:
	sub si, si
	mov byte [CTRL_REQUEST], REQUEST_READ

int13h_func_read_1:
	cmp byte [CTRL_REQUEST], REQUEST_DONE
	jne int13h_func_read_1

	mov cx, DATA_BYTES
	rep movsb

	dec al
	jnz int13h_func_read_2

	mov byte ah, [CTRL_STATUS]
	call int13h_set_status

	pop si
	pop di
	pop cx
	jmp int13_iret

int13h_func_write:
%ifdef TRACE
	DisplayStringLine "write!"
%endif
	mov ah, 0x3
	call int13h_set_status
	jmp int13_iret

	; Status in AH
int13h_set_status:
	push ds
	push ax
	mov ax, 0x40
	mov ds, ax
	pop ax
	mov [0x41], ah
	pop ds
	ret

	; Status in AL
int13h_get_status:
	push ds
	push ax
	mov ax, 0x40
	mov ds, ax
	pop ax
	mov al, [0x41]
	pop ds
	ret

int13_iret:
	cmp ah, 0
	je int13_iret_clc
	jmp int13_iret_stc

int13_iret_clc:
	pop ds
	pop bp
	popf
	clc
	retf 2

int13_iret_stc:
	pop ds
	pop bp
	popf
	stc
	retf 2	

install_ipl_diskette:
	push ds
	push ax
	mov ax, 0x40
	mov ds, ax
	mov ax, [0x10]
	or ax, 0x1
	mov [0x10], ax
	pop ax
	pop ds
	ret

install_int13h:
	push ax
	push bx
	push ds

	xor ax, ax
	mov ds, ax

	mov word [0x13 * 4], int13h
	mov [0x13 * 4 + 2], cs

	pop ds
	pop bx
	pop ax
	ret

delay:
	push ax
	mov ax, 0xffff
.loop:
	dec ax
	nop
	nop
	nop
	jnz .loop
	pop ax
	ret

	; String address in DS:SI
display_string:
	push ax
	push bx
	mov bx, si
	
display_string_1:
	mov al,[bx]
	cmp al,0
	jz display_string_2
	call display_char
	inc bx
	jmp display_string_1

display_string_2:
	pop bx
	pop ax
	ret

	; Char in AL
display_char:
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	mov ah, 0x0e
	mov bx,0x000f
	int 0x10
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	ret

	; Char in AL
read_char:
	push bx
	push cx
	push dx
	push si
	push di
	mov ah, 0x00
	int 0x16
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	ret
