    .global font
    .global font_size
    .section .rodata
font:
    .incbin "../font/MStiffHei PRC Black.ttf"
1:
font_size:
    .int 1b - font
