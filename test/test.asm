cpu 8086
org 100h

%macro DisplayString 1
	[section .data]
	
	%%str: db %1, 0

	__SECT__

	push si
	mov si, %%str
	call display_string
	pop si
%endmacro

section .bss

buffer_size: equ 0x8000 ; 32K
buffer: resb buffer_size

section .text

main:
	DisplayString `Poisk HDD test version 0.1\r\n`
	DisplayString `Copyright (c) 2023 Peter Hizalev\r\n\r\n`

.repeat:
	mov ah, 0 ; reset
	mov dl, 0x80
	int 0x13
	jc .error

	DisplayString `Press [R]ead, [W]rite or [V]erify: `
	call read_char
	call display_char
	DisplayString `\r\n`

	cmp al, 'r'
	je .read

	cmp al, 'R'
	je .read

	cmp al, 'w'
	je .write

	cmp al, 'W'
	je .write

	cmp al, 'v'
	je .verify

	cmp al, 'V'
	je .verify

	jmp .exit

.read:
	mov ah, 0x2
	jmp .set_params

.write:
	cld
	mov cx, buffer_size
	mov di, buffer
	mov al, 0xad
	rep stosb

	mov ah, 0x3
	jmp .set_params

.verify:
	mov ah, 0x4
	jmp .set_params

.set_params:
	mov al, 0x40 ; 64 sectors
	mov ch, 0xff ;
	mov cl, 0xc1 ; cylinder 1023, sector 1
	mov dh, 0    ; head 0
	mov dl, 0x80 ; hard disk 0
	mov bx, buffer
	int 0x13
	jc .error

	DisplayString `Success!\r\n`
	jmp .repeat

.error:
	DisplayString `Error\r\n`

.exit:
	int 0x20

	; Word in AX
display_word:
	push ax
	mov al, ah
	call display_byte
	pop ax
	call display_byte	
	ret

	; Double word in DX:AX
display_dword:
	push dx
	push ax
	mov al, dh
	call display_byte
	mov al, dl
	call display_byte
	mov al, ah
	call display_byte
	pop ax
	call display_byte	
	pop dx
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
