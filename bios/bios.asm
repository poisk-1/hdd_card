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

%macro InstallISR 2
	push ax
	push bx
	push ds

	xor ax, ax
	mov ds, ax

	mov word [%1 * 4], %2
	mov [%1 * 4 + 2], cs

	pop ds
	pop bx
	pop ax
%endmacro

boot_sector_offset equ 0x7c00
boot_sector_size equ 0x200
boot_sector_signature_offset equ (boot_sector_offset + boot_sector_size - 2)
boot_sector_signature equ 0xaa55

ctrl_request_done:                  equ 0x00
ctrl_request_check:                 equ 0x01
ctrl_request_scan:                  equ 0x02
ctrl_request_reset:                 equ 0x03
ctrl_request_read:                  equ 0x04
ctrl_request_read_next:             equ 0x05
ctrl_request_write:                 equ 0x06
ctrl_request_write_next:            equ 0x07
ctrl_request_verify:                equ 0x08
ctrl_request_verify_next:           equ 0x09
ctrl_request_read_params_fun8h:     equ 0x0a
ctrl_request_read_params_fun15h:    equ 0x0b
ctrl_request_detect_media_change:   equ 0x0c

io_offset: equ 0x1000 ; reserve 4K for BIOS

struc io_data, io_offset
	.buffer resb 0x200
endstruc

struc io_ctrl, io_offset + io_data_size
	.request resb 1
endstruc

io_ctrl_req_offset: equ io_offset + io_data_size + io_ctrl_size

struc io_ctrl_drive_req, io_ctrl_req_offset
	.status resb 1
	.low_cylinder_number resb 1
	.sector_and_high_cylinder_numbers resb 1
	.head_number resb 1
	.drive_number resb 1
endstruc

struc io_ctrl_scan_req, io_ctrl_req_offset
	.number_of_floppy_drives resb 1
	.number_of_hard_drives resb 1
endstruc

struc io_ctrl_read_params_fun8h_req, io_ctrl_req_offset
    .drive_number resb 1

    .success resb 1

    .drive_type resb 1
    .max_low_cylinder_number resb 1
    .max_sector_and_high_cylinder_numbers resb 1
    .max_head_number resb 1
    .number_of_drives resb 1
endstruc

struc io_ctrl_read_params_fun15h_req, io_ctrl_req_offset
    .drive_number resb 1

    .success resb 1

    .drive_type resb 1
endstruc

struc io_ctrl_detect_media_change_req, io_ctrl_req_offset
    .drive_number resb 1

    .status resb 1
endstruc

%macro SubmitIo 1
	mov byte [io_ctrl.request], %1

	cmp byte [io_ctrl.request], ctrl_request_done
	je %%done

	push cx
	mov cx, 0x100
%%wait:
	push cx
%%delay:
	loop %%delay
	pop cx
	cmp cx, 0x10
	je %%min_delay
	shr cx, 1
%%min_delay:
	cmp byte [io_ctrl.request], ctrl_request_done
	jne %%wait
	pop cx
%%done:
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
	DisplayString `Copyright (c) 2022-2023 Peter Hizalev\r\n\r\n`

	mov ax, cs
	mov ds, ax ; DS to point to IO memory
	mov es, ax ; ES to point to IO memory

	mov cx, io_data_size
	mov di, io_data

	SubmitIo ctrl_request_check

.fill:
	mov ax, io_data_size
	sub ax, cx
	stosb
	loop .fill

	SubmitIo ctrl_request_check

	mov cx, io_data_size
	mov si, io_data

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
	call delay
	jmp .done

.check_succeeded:
	DisplayString `Adapter successfully initialized\r\nScanning disk drives...\r\n`

	SubmitIo ctrl_request_scan
	mov byte al, [io_ctrl_scan_req.number_of_floppy_drives] ; 0-4
	mov byte ah, [io_ctrl_scan_req.number_of_hard_drives] ; 0-2

	push ax

	call display_nibble
	DisplayString ` floppy disk drive(s) found\r\n`

	mov al, ah
	call display_nibble
	DisplayString ` hard disk drive(s) found\r\n`

	pop ax

	call install_bios_data
	InstallISR 0x19, int19h
	InstallISR 0x13, int13h

.done:
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
	push dx
	push di
	push si

	mov di, cs
	mov ds, di ; DS to point to IO memory

	cmp ah, 0x1f
	jg .unsupported

	mov di, ax
	xor al, al
	xchg al, ah
	shl ax, 1
	xchg ax, di	
	add di, .dispatch

	cmp word [cs:di], 0
	je .unsupported

	jmp [cs:di]

.unsupported:
	mov al, ah
	call display_byte
	DisplayString `!!!\r\n`

	mov ah, 0x1 ; invalid function
	call set_status

	jmp .return_error
	
.dispatch:
	dw .reset                 ; 00
	dw .read_status	          ; 01
	dw .read                  ; 02
	dw .write                 ; 03
	dw .verify                ; 04
	dw .empty                 ; 05 (format)
	dw 0                      ; 06
	dw 0                      ; 07
	dw .read_params_fun8h     ; 08
	dw 0                      ; 09
	dw 0                      ; 0a
	dw 0                      ; 0b
	dw .empty                 ; 0c (seek)
	dw 0                      ; 0d
	dw 0                      ; 0e
	dw 0                      ; 0f
	dw 0                      ; 10
	dw 0                      ; 11
	dw 0                      ; 12
	dw 0                      ; 13
	dw 0                      ; 14
	dw .read_params_fun15h    ; 15
	dw .detect_media_change   ; 16
	dw .empty                 ; 17 (set diskette type)
	dw .empty                 ; 18 (set media type)
	dw 0                      ; 19
	dw 0                      ; 1a
	dw 0                      ; 1b
	dw 0                      ; 1c
	dw 0                      ; 1d
	dw 0                      ; 1e
	dw 0                      ; 1f

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

	mov byte [io_ctrl_drive_req.low_cylinder_number], ch
	mov byte [io_ctrl_drive_req.sector_and_high_cylinder_numbers], cl
	mov byte [io_ctrl_drive_req.head_number], dh
	mov byte [io_ctrl_drive_req.drive_number], dl

	mov bl, al ; save number of sectors to read

.read_next_sector:
	cmp bl, al ; submit read next?
	jne .do_read_next

	SubmitIo ctrl_request_read
	
	jmp .check_read_status

.do_read_next:
	SubmitIo ctrl_request_read_next

.check_read_status:
	mov byte ah, [io_ctrl_drive_req.status]

	call set_status

	cmp ah, 0 ; has read error occured?
	jne .read_error

	mov si, io_data ; make SI to point to the beginning of the IO buffer
	mov cx, io_data_size
	rep movsb ; copy data bytes from IO buffer (DS:SI) to read buffer (ES:DI)

	dec al
	jnz .read_next_sector

	mov al, bl ; all sectors read

	jmp .return_success

.read_error:
	sub bl, al ; some sectors read
	mov al, bl

	jmp .return_error

	; AL - number of sectors to write
	; CH, CL, DH, DL - CSHD address of first sector
	; ES:BX - pointer to the write buffer
	;
	; Returns:
	; AH - status
.write:
	mov si, bx ; make SI point to the beginning of the write buffer

	mov byte [io_ctrl_drive_req.low_cylinder_number], ch
	mov byte [io_ctrl_drive_req.sector_and_high_cylinder_numbers], cl
	mov byte [io_ctrl_drive_req.head_number], dh
	mov byte [io_ctrl_drive_req.drive_number], dl

	mov bl, al ; save number of sectors to write

.write_next_sector:
	mov di, io_data ; make DI to point to the beginning of the IO buffer
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

	cmp bl, al ; submit write next?
	jne .do_write_next

	SubmitIo ctrl_request_write

	jmp .check_write_status

.do_write_next:
	SubmitIo ctrl_request_write_next

.check_write_status:
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

	jmp .return_error

.verify:
	mov byte [io_ctrl_drive_req.low_cylinder_number], ch
	mov byte [io_ctrl_drive_req.sector_and_high_cylinder_numbers], cl
	mov byte [io_ctrl_drive_req.head_number], dh
	mov byte [io_ctrl_drive_req.drive_number], dl

	mov bl, al ; save number of sectors to verify

.verify_next_sector:
	cmp bl, al ; submit read next?
	jne .do_verify_next

	SubmitIo ctrl_request_verify

	jmp .check_verify_status

.do_verify_next:
	SubmitIo ctrl_request_verify_next

.check_verify_status:
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

	jmp .return_error

.empty:
	mov ah, 0
	call set_status
	jmp .return_success

	; DL - drive number
	; Returns:
	; AX - 0
	; BH - 0
	; BL, CH, CL, DH - drive info
	; DL - number of drives
.read_params_fun8h:
	mov byte [io_ctrl_read_params_fun8h_req.drive_number], dl

	SubmitIo ctrl_request_read_params_fun8h

	pop si
	pop di
	pop dx
	pop cx
	pop bx

	mov ax, 0
	mov bh, 0
	mov byte bl, [io_ctrl_read_params_fun8h_req.drive_type]
	mov byte ch, [io_ctrl_read_params_fun8h_req.max_low_cylinder_number]
	mov byte cl, [io_ctrl_read_params_fun8h_req.max_sector_and_high_cylinder_numbers]
	mov byte dh, [io_ctrl_read_params_fun8h_req.max_head_number]
	mov byte dl, [io_ctrl_read_params_fun8h_req.number_of_drives]

	cmp byte [io_ctrl_read_params_fun8h_req.success], 0
	jne .return_success_and_registers
	jmp .return_error_and_registers

	; DL - drive number
	; Returns:
	; AH - drive type
.read_params_fun15h:
	mov byte [io_ctrl_read_params_fun15h_req.drive_number], dl

	SubmitIo ctrl_request_read_params_fun15h

	mov byte ah, [io_ctrl_read_params_fun15h_req.drive_type]

	cmp byte [io_ctrl_read_params_fun15h_req.success], 0
	jne .return_success
	jmp .return_error

	; DL - drive number
	; Returns:
	; AH - media change status
.detect_media_change:
	mov byte [io_ctrl_detect_media_change_req.drive_number], dl

	SubmitIo ctrl_request_detect_media_change

	mov byte ah, [io_ctrl_detect_media_change_req.status]

	call set_status

	cmp ah, 0 ; media changed or error?
	jne .return_error
	jmp .return_success

.return_success:
	pop si
	pop di
	pop dx
	pop cx
	pop bx
.return_success_and_registers:
	pop ds
	popf
	clc
	retf 2

.return_error:
	pop si
	pop di
	pop dx
	pop cx
	pop bx
.return_error_and_registers:
	pop ds
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

	; AL - number of floppy drives 0-4
	; AH - number of hard drives 0-2
install_bios_data:
	push ds
	push ax
	push bx
	push cx

	mov bx, 0x40
	mov ds, bx	

	mov [0x75], ah ; number of hard drives

	cmp al, 0
	je .done

	dec al
	mov cl, 6
	shl al, cl

	mov bx, [0x10]
	and bl, 0x3f
	or bl, al ; number of floppy drives minus 1 in bits 7-6
	or bl, 1  ; boot from floppy in bit 0
	mov [0x10], bx

.done:
	pop cx
	pop bx
	pop ax
	pop ds
	ret

int19h:
	pushf
	push ds
	push es
	push ax
	push bx
	push cx
	sti
	cld	

	mov ax, 0x40
	mov ds, ax

	mov ax, 0
	mov es, ax
	mov bx, boot_sector_offset ; boot sector at [ES:BX] = [0000:7c00]

	mov ah, [0x10] ; boot from floppy in bit 0
	test ah, 1
	jz .try_hard_drive

	mov dl, 0 ; boot from 1st floppy drive

.try_floppy_drive:
	call .read_boot_sector
	jc .try_next_floppy_drive

	cmp word [es:boot_sector_signature_offset], boot_sector_signature
	jne .try_next_floppy_drive

	DisplayString `Floppy drive #`
	mov al, dl
	call display_nibble
	DisplayString ` has bootable disk. Would you like to boot from it? [y/n]: `
	call read_char
	call display_char
	DisplayString `\r\n`

	cmp al, 'y'
	je .boot

	cmp al, 'Y'
	je .boot

.try_next_floppy_drive:
	inc dl
	cmp dl, 4 ; try all 4 floppy drives
	jl .try_floppy_drive

.try_hard_drive:
	mov ah, [0x75] ; number of hard drives
	cmp ah, 0
	je .no_boot

	mov dl, 0x80 ; boot from 1st hard drive
	call .read_boot_sector
	jc .no_boot

	cmp word [es:boot_sector_signature_offset], boot_sector_signature
	jne .no_boot

	DisplayString `Hard disk is bootable.\r\n`

	jmp .boot

.no_boot:
	DisplayString `No bootable disk was found!\r\n`
	call delay

	pop cx	
	pop bx
	pop ax
	pop es
	pop ds
	popf
	retf 2

.boot:
	DisplayString `Booting an operating system ...\r\n\r\n`

	jmp 0:0x7c00

	; DL - drive number
	; ES:BX - memory location
.read_boot_sector:
	mov cx, 4 ; number of retries
.retry_read:
	push cx

	mov ah, 0 ; reset drive
	int 0x13
	jc .reset_error

	mov ah, 2 ; read sector
	mov cx, 1 ; track 0, sector 1
	mov dh, 0 ; head 0
	mov al, 1 ; read 1 sector
	int 0x13

.reset_error:
	pop cx
	jnc .return_success
	loop .retry_read
.return_success:
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
