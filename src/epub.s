    .global font
    .global font_size
    .global style
    .global style_size
    .section .rodata

font:
    .incbin "SourceHanSansHWSC-Bold.otf"
font_end:
font_size:
    .int font_end - font

style:
    .incbin "style.css"
style_end:
style_size:
    .int style_end - style
