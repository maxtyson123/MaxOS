.section .mb2_hdr

# multiboot2 header: magic number, mode, length, checksum
mb2_hdr_begin:
.long 0xE85250D6
.long 0
.long (mb2_hdr_end - mb2_hdr_begin)
.long -(0xE85250D6 + (mb2_hdr_end - mb2_hdr_begin))

# framebuffer tag: type = 5
mb2_framebuffer_req:
    .short 5
    .short 1
    .long (mb2_framebuffer_end - mb2_framebuffer_req)
    # preferred width, height, bpp.
    # leave as zero to indicate "don't care"
    .long 0
    .long 0
    .long 0
mb2_framebuffer_end:

# the end tag: type = 0, size = 8
.long 0
.long 8
mb2_hdr_end: