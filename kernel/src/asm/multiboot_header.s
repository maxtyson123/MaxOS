section .multiboot_header
header_start:
	align 8
    dd 0xe85250d6                   ; Magic number
    dd 0                            ; Protected mode
    dd header_end - header_start    ; Header length

    ; Checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

framebuffer_tag_start:
    dw  0x05                                            ; Tag: Framebuffer
    dw  0x01                                            ; Tag is optional
    dd  framebuffer_tag_end - framebuffer_tag_start     ; Size
    dd  0                                               ; Width  - let GRUB pick
    dd  0                                               ; Height - let GRUB pick
    dd  0                                               ; Depth  - let GRUB pick
framebuffer_tag_end:

    ; End Of multiboot tag
    align 8
    dw 0    ;type
    dw 0    ;flags
    dd 8    ;size
header_end:

