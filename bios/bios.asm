cpu 8086
org 0

%macro DisplayString 1
	[section .data]
	
	%%str: db %1, 0

	__SECT__

	push si
	mov si, %%str
	call display_string
	pop si
%endmacro

%macro SubmitIo 1
	mov byte [io.ctrl.request], %1

%%wait:
	cmp byte [io.ctrl.request], ctrl_request_done
	jne %%wait

	mov byte ah, [io.ctrl.status]
%endmacro

ctrl_request_done: equ 0x00
ctrl_request_init: equ 0x01
ctrl_request_read: equ 0x02
ctrl_request_write: equ 0x03

io_segment: equ 0xe000

io_data_size_bytes: equ 0x200

struc io
	.data: resb io_data_size_bytes

	.ctrl.request resb 1
	.ctrl.status resb 1

	.ctrl.cylinder_number resb 1
	.ctrl.sector_number resb 1
	.ctrl.head_number resb 1
	.ctrl.drive_number resb 1
endstruc

section .text

	db 0x55, 0xaa ; BIOS marker
	db 0 ; BIOS size / 512 bytes

	push ax
	push bx
	push cx
	push dx
	push ds

	mov ax, cs
	mov ds, ax

	call cls

	DisplayString `Poisk HDD card version 0.1\r\n`
	DisplayString `Copyright (c) 2022 Peter Hizalev\r\n`

	call install_ipl_diskette
	call install_int13h

	call delay
	call cls

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
	push ds
	push bx
	push cx
	push di
	push si

	mov di, io_segment
	mov ds, di ; DS to point to IO memory

	cmp ah, 0x3
	jg .return_error

	mov di, ax
	xor al, al
	xchg al, ah
	shl ax, 1
	xchg ax, di
	add di, .table

	jmp [cs:di]
	
.table:
	dw .reset
	dw .get_status
	dw .read
	dw .write

	; Returns:
	; AH - status
.reset:
	SubmitIo ctrl_request_init

	call set_status

	cmp ah, 0 ; has reset error occured?
	jne .return_error
	jmp .return_success

	; Returns:
	; AH - status
.get_status:
	call get_status

	jmp .return_success

	; AL - number of sectors
	; CH, CL, DH, DL - CSHD address of first sector
	; ES:BX - pointer to buffer
	;
	; Returns:
	; AH - status
.read:

	mov di, bx ; make ES:DI point to the read buffer

	mov byte [io.ctrl.cylinder_number], ch
	mov byte [io.ctrl.sector_number], cl
	mov byte [io.ctrl.head_number], dh
	mov byte [io.ctrl.drive_number], dl

	mov bl, al ; save number of sectors to read

.read_next_sector:
	sub si, si ; make DS:SI to point to beginning of the IO buffer

	SubmitIo ctrl_request_read

	call set_status

	cmp ah, 0 ; has read error occured?
	jne .read_error

	mov cx, io_data_size_bytes
	rep movsb ; copy data bytes from IO buffer to read buffer

	dec al
	jnz .read_next_sector

	mov al, bl ; all sectors read

	jmp .return_success

.read_error:
	sub bl, al ; some sectors read
	mov al, bl

	jne .return_error

.write:
	mov ah, 0x3
	call set_status
	jmp .return_error

.return_success:
	pop si
	pop di
	pop cx
	pop bx
	pop ds
	popf
	clc
	retf 2

.return_error:
	pop si
	pop di
	pop cx
	pop ds
	pop bp
	popf
	stc
	retf 2	

	; Status in AH
set_status:
	push ds
	push ax
	mov ax, 0x40
	mov ds, ax
	pop ax
	mov [0x41], ah
	pop ds
	ret

	; Returns:
	; Status in AL
get_status:
	push ds
	push ax
	mov ax, 0x40
	mov ds, ax
	pop ax
	mov al, [0x41]
	pop ds
	ret

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
	push cx
	mov cx, 0xffff
.continue:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	loop .continue
	pop cx
	ret

	; String address in DS:SI
display_string:
	push ax
	push bx
	mov bx, si
	
.next_char:
	mov al,[bx]
	cmp al,0
	jz .done
	call display_char
	inc bx
	jmp .next_char

.done:
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

cls:
	push ax
	mov ah, 0
	mov al, 3
	int 10h
	pop ax
	ret
