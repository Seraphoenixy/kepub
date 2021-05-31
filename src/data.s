    .global container
    .global container_size
    .global FZCYSJW
    .global FZCYSJW_size
    .global style
    .global style_size
    .global mimetype
    .global mimetype_size
    .global content
    .global content_size
    .global toc
    .global toc_size
    .global chapter
    .global chapter_size
    .global cover
    .global cover_size
    .global introduction
    .global introduction_size
    .global illustration
    .global illustration_size
    .global message
    .global message_size
    .global postscript
    .global postscript_size
    .section .rodata

container:
    .incbin "container.xml"
container_end:
container_size:
    .int container_end - container

FZCYSJW:
    .incbin "FZCYSJW.ttf"
FZCYSJW_end:
FZCYSJW_size:
    .int FZCYSJW_end - FZCYSJW

style:
    .incbin "style.css"
style_end:
style_size:
    .int style_end - style

mimetype:
    .incbin "mimetype"
mimetype_end:
mimetype_size:
    .int mimetype_end - mimetype

content:
    .incbin "content.opf"
content_end:
content_size:
    .int content_end - content

toc:
    .incbin "toc.ncx"
toc_end:
toc_size:
    .int toc_end - toc

chapter:
    .incbin "chapter.xhtml"
chapter_end:
chapter_size:
    .int chapter_end - chapter

cover:
    .incbin "cover.xhtml"
cover_end:
cover_size:
    .int cover_end - cover

introduction:
    .incbin "introduction.xhtml"
introduction_end:
introduction_size:
    .int introduction_end - introduction

illustration:
    .incbin "illustration.xhtml"
illustration_end:
illustration_size:
    .int illustration_end - illustration

message:
    .incbin "message.xhtml"
message_end:
message_size:
    .int message_end - message

postscript:
    .incbin "postscript.xhtml"
postscript_end:
postscript_size:
    .int postscript_end - postscript
