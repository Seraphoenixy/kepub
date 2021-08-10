    .global style
    .global style_size
    .section .rodata

style:
    .incbin "style.css"
style_end:
style_size:
    .int style_end - style
