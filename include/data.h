#pragma once

#include <string>
#include <string_view>

extern char container[];
extern int container_size;

extern char DFNMing[];
extern int DFNMing_size;

extern char DFPMingLight[];
extern int DFPMingLight_size;

extern char FZCYS[];
extern int FZCYS_size;

extern char SourceHanSansCN_Normal[];
extern int SourceHanSansCN_Normal_size;

extern char style[];
extern int style_size;

extern char mimetype[];
extern int mimetype_size;

extern char content[];
extern int content_size;

extern char toc[];
extern int toc_size;

extern char chapter[];
extern int chapter_size;

extern char cover[];
extern int cover_size;

extern char introduction[];
extern int introduction_size;

namespace kepub {

inline std::string_view container_str(
    container, static_cast<std::string_view::size_type>(container_size));

inline std::string_view DFNMing_str(
    DFNMing, static_cast<std::string_view::size_type>(DFNMing_size));

inline std::string_view DFPMingLight_str(
    DFPMingLight, static_cast<std::string_view::size_type>(DFPMingLight_size));

inline std::string_view FZCYS_str(
    FZCYS, static_cast<std::string_view::size_type>(FZCYS_size));

inline std::string_view SourceHanSansCN_Normal_str(
    SourceHanSansCN_Normal,
    static_cast<std::string_view::size_type>(SourceHanSansCN_Normal_size));

inline std::string_view style_str(
    style, static_cast<std::string_view::size_type>(style_size));

inline std::string_view mimetype_str(
    mimetype, static_cast<std::string_view::size_type>(mimetype_size));

inline std::string content_str(
    content, static_cast<std::string::size_type>(content_size));

inline std::string toc_str(toc, static_cast<std::string::size_type>(toc_size));

inline std::string chapter_str(
    chapter, static_cast<std::string::size_type>(chapter_size));

inline std::string_view cover_str(
    cover, static_cast<std::string_view::size_type>(cover_size));

inline std::string introduction_str(
    introduction, static_cast<std::string::size_type>(introduction_size));

}  // namespace kepub
