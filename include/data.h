#pragma once

#include <string>
#include <string_view>

extern char container[];
extern int container_size;

extern char FZCYSJW[];
extern int FZCYSJW_size;

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

extern char illustration[];
extern int illustration_size;

extern char message[];
extern int message_size;

extern char postscript[];
extern int postscript_size;

namespace kepub {

inline std::string_view container_str(
    container, static_cast<std::string_view::size_type>(container_size));

inline std::string_view FZCYSJW_str(
    FZCYSJW, static_cast<std::string_view::size_type>(FZCYSJW_size));

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

inline std::string illustration_str(
    illustration, static_cast<std::string::size_type>(illustration_size));

inline std::string message_str(
    message, static_cast<std::string::size_type>(message_size));

inline std::string_view postscript_str(
    postscript, static_cast<std::string_view::size_type>(postscript_size));

}  // namespace kepub
