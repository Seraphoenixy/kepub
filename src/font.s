    .global font
    .global font_size
    .section .rodata

font:
    .incbin "SourceHanSansHWSC-Bold.otf"
font_end:
font_size:
    .int font_end - font
