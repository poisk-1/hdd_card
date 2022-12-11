cpu 8086
org 0

%macro DisplayString 1
	[section .data]
	
	%%str: db %1, 0

	__SECT__

	push ds
	push si
	mov si, cs
	mov ds, si
	mov si, %%str
	call display_string
	pop si
	pop ds
%endmacro

ctrl_request_done: equ 0x00
ctrl_request_check: equ 0x01
ctrl_request_scan: equ 0x02
ctrl_request_reset: equ 0x03
ctrl_request_read: equ 0x04
ctrl_request_write: equ 0x05
ctrl_request_verify: equ 0x06
ctrl_request_format: equ 0x07

io_segment: equ 0xe000
io_data_size: equ 0x200

struc io_ctrl, io_data_size
	.request resb 1
endstruc

struc io_ctrl_drive_req, (io_data_size + io_ctrl_size)
	.status resb 1
	.cylinder_number resb 1
	.sector_number resb 1
	.head_number resb 1
	.drive_number resb 1
endstruc

struc io_ctrl_scan_req, (io_data_size + io_ctrl_size)
	.number_of_floppy_drives resb 1
	.number_of_hard_drives resb 1
endstruc

%macro SubmitIo 1
	mov byte [io_ctrl.request], %1

%%wait:
	cmp byte [io_ctrl.request], ctrl_request_done
	jne %%wait
%endmacro

section .text

	db 0x55, 0xaa ; BIOS marker
	db 0 ; BIOS size / 512 bytes

entry:
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	push ds
	push es

	call cls

	DisplayString `Poisk HDD card version 0.1\r\n`
	DisplayString `Copyright (c) 2022 Peter Hizalev\r\n\r\n`

	mov ax, io_segment
	mov ds, ax ; DS to point to IO memory
	mov es, ax ; ES to point to IO memory

	mov cx, io_data_size
	sub di, di

.fill:
	mov ax, io_data_size
	sub ax, cx
	stosb
	loop .fill

	SubmitIo ctrl_request_check

	mov cx, io_data_size
	sub si, si

.check:
	mov bx, io_data_size
	sub bx, cx
	lodsb
	not al
	cmp al, bl
	jne .check_failed
	loop .check
	jmp .check_succeeded

.check_failed:
	DisplayString `Adapter failure!\r\n`
	jmp .done

.check_succeeded:
	DisplayString `Adapter found\r\nScanning drives...\r\n`

	SubmitIo ctrl_request_scan
	mov byte al, [io_ctrl_scan_req.number_of_floppy_drives]
	mov byte ah, [io_ctrl_scan_req.number_of_hard_drives]

	push ax

	call display_byte
	DisplayString ` floppy drive(s) found\r\n`

	mov al, ah
	call display_byte
	DisplayString ` hard drive(s) found\r\n`

	pop ax

	DisplayString `Installing INT 13h...\r\n`

	call install_ipl_diskette
	call install_int13h

.done:
	;call delay
	;call cls

	pop es
	pop ds
	pop di
	pop si
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

	cmp ah, 0x4
	jg .unsupported

	mov di, ax
	xor al, al
	xchg al, ah
	shl ax, 1
	xchg ax, di
	add di, .dispatch

	jmp [cs:di]

.unsupported:
	mov al, ah
	call display_byte
	DisplayString `!!!\r\n`
	jmp .return_error
	
.dispatch:
	dw .reset
	dw .read_status
	dw .read
	dw .write
	dw .verify

	; DL - drive number
	; Returns:
	; AH - status
.reset:
	mov byte [io_ctrl_drive_req.drive_number], dl

	SubmitIo ctrl_request_reset
	mov byte ah, [io_ctrl_drive_req.status]

	call set_status

	cmp ah, 0 ; has reset error occured?
	jne .return_error
	jmp .return_success

	; Returns:
	; AH - status
.read_status:
	call get_status

	jmp .return_success

	; AL - number of sectors to read
	; CH, CL, DH, DL - CSHD address of first sector
	; ES:BX - pointer to the read buffer
	;
	; Returns:
	; AH - status
.read:

	mov di, bx ; make DI point to the beginning of the read buffer

	mov byte [io_ctrl_drive_req.cylinder_number], ch
	mov byte [io_ctrl_drive_req.sector_number], cl
	mov byte [io_ctrl_drive_req.head_number], dh
	mov byte [io_ctrl_drive_req.drive_number], dl

	mov bl, al ; save number of sectors to read

.read_next_sector:
	sub si, si ; make SI to point to the beginning of the IO buffer

	SubmitIo ctrl_request_read
	mov byte ah, [io_ctrl_drive_req.status]

	call set_status

	cmp ah, 0 ; has read error occured?
	jne .read_error

	mov cx, io_data_size
	rep movsb ; copy data bytes from IO buffer (DS:SI) to read buffer (ES:DI)

	dec al
	jnz .read_next_sector

	mov al, bl ; all sectors read

	jmp .return_success

.read_error:
	sub bl, al ; some sectors read
	mov al, bl

	jne .return_error

	; AL - number of sectors to write
	; CH, CL, DH, DL - CSHD address of first sector
	; ES:BX - pointer to the write buffer
	;
	; Returns:
	; AH - status
.write:
	mov si, bx ; make SI point to the beginning of the write buffer

	mov byte [io_ctrl_drive_req.cylinder_number], ch
	mov byte [io_ctrl_drive_req.sector_number], cl
	mov byte [io_ctrl_drive_req.head_number], dh
	mov byte [io_ctrl_drive_req.drive_number], dl

	mov bl, al ; save number of sectors to write

.write_next_sector:
	sub di, di ; make DI to point to the beginning of the IO buffer

	mov cx, io_data_size

	push es
	push ds
	pop es
	pop ds ; swap DS and ES

	rep movsb ; copy data bytes from write buffer (DS:SI) to IO buffer (ES:DI)

	push es
	push ds
	pop es
	pop ds ; swap DS and ES

	SubmitIo ctrl_request_write
	mov byte ah, [io_ctrl_drive_req.status]

	call set_status

	cmp ah, 0 ; has read error occured?
	jne .write_error

	dec al
	jnz .write_next_sector

	mov al, bl ; all sectors read

	jmp .return_success

.write_error:
	sub bl, al ; some sectors read
	mov al, bl

	jne .return_error

	; AL - number of sectors to verify
	; CH, CL, DH, DL - CSHD address of first sector
	;
	; Returns:
	; AH - status
.verify:

	mov byte [io_ctrl_drive_req.cylinder_number], ch
	mov byte [io_ctrl_drive_req.sector_number], cl
	mov byte [io_ctrl_drive_req.head_number], dh
	mov byte [io_ctrl_drive_req.drive_number], dl

	mov bl, al ; save number of sectors to verify

.verify_next_sector:
	SubmitIo ctrl_request_verify
	mov byte ah, [io_ctrl_drive_req.status]

	call set_status

	cmp ah, 0 ; has verification error occured?
	jne .verify_error

	dec al
	jnz .verify_next_sector

	mov al, bl ; all sectors verified

	jmp .return_success

.verify_error:
	sub bl, al ; some sectors verified
	mov al, bl

	jne .return_error

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

	; AL - number of floppy drives
	; AH - number of hard drives
install_ipl_diskette:
	push ds
	push ax
	push bx
	push cx

	mov bx, 0x40
	mov ds, bx	

	mov cl, 6
	shl al, cl ; number of floppy drives in bits 7-6

	mov bx, [0x10]
	and bl, 0x3f
	or bl, al

	cmp al, 0
	je .no_boot
	or bl, 0x1 ; boot from floppy in bit 0

.no_boot:
	mov [0x10], bx

	pop cx
	pop bx
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

	; Byte in AL
display_byte:
	push cx
	push ax
	mov cl, 4
	shr al, cl
	call display_nibble
	pop ax
	push ax
	and al, 0x0f
	call display_nibble
	pop ax
	pop cx
	ret

	; 4 LSB in AL
display_nibble:
	push ax
	and al, 0x0f
	cmp al, 0x09
	jle .is_digit
	add al, 0x07
.is_digit:
	add al, 0x30
	call display_char
	pop ax
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
