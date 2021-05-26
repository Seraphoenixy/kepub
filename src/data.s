    .global container
    .global container_size
    .global DFNMing
    .global DFNMing_size
    .global DFPMingLight
    .global DFPMingLight_size
    .global FZCYS
    .global FZCYS_size
    .global SourceHanSansCN_Normal
    .global SourceHanSansCN_Normal_size
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
    .section .rodata

container:
    .incbin "container.xml"
container_end:
container_size:
    .int container_end - container

DFNMing:
    .incbin "DFNMing.ttf"
DFNMing_end:
DFNMing_size:
    .int DFNMing_end - DFNMing

DFPMingLight:
    .incbin "DFPMingLight.ttf"
DFPMingLight_end:
DFPMingLight_size:
    .int DFPMingLight_end - DFPMingLight

FZCYS:
    .incbin "FZCYS.ttf"
FZCYS_end:
FZCYS_size:
    .int FZCYS_end - FZCYS

SourceHanSansCN_Normal:
    .incbin "SourceHanSansCN_Normal.ttf"
SourceHanSansCN_Normal_end:
SourceHanSansCN_Normal_size:
    .int SourceHanSansCN_Normal_end - SourceHanSansCN_Normal

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
