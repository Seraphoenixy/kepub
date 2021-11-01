#include <filesystem>
#include <memory>

#include <fmt/format.h>
#include <klib/util.h>
#include <catch2/catch.hpp>

#include "epub.h"

TEST_CASE("base generate", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book1");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book1"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book1");

  REQUIRE(std::filesystem::exists(kepub::Epub::container_path));
  REQUIRE(klib::read_file(kepub::Epub::container_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
    <rootfiles>
        <rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml" />
    </rootfiles>
</container>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::font_path));
  REQUIRE(std::filesystem::file_size(kepub::Epub::font_path) == 17002512);

  REQUIRE(std::filesystem::exists(kepub::Epub::style_path));
  REQUIRE(std::filesystem::file_size(kepub::Epub::style_path) == 5146);

  REQUIRE(std::filesystem::exists(kepub::Epub::introduction_path));
  REQUIRE(klib::read_file(kepub::Epub::introduction_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>简介</title>
    </head>
    <body>
        <div>
            <h1 class="bold">简介</h1>
            <p>test</p>
            <p>introduction</p>
        </div>
    </body>
</html>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::message_path));
  REQUIRE(klib::read_file(kepub::Epub::message_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>制作信息</title>
    </head>
    <body>
        <div>
            <h1 class="bold">制作信息</h1>
            <div class="cutline">
                <p class="makerifm">制作者：kaiser</p>
            </div>
        </div>
    </body>
</html>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book1</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
    </spine>
    <guide />
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book1</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
    </navMap>
</ncx>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::mimetype_path));
  REQUIRE(klib::read_file(kepub::Epub::mimetype_path, false) ==
          R"(application/epub+zip
)");

  ptr.reset();

  std::filesystem::remove_all("test book1");
}

TEST_CASE("generate postscript", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book2");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});
  epub.set_generate_postscript(true);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book2"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book2");

  REQUIRE(std::filesystem::exists(kepub::Epub::postscript_path));
  REQUIRE(klib::read_file(kepub::Epub::postscript_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>后记</title>
    </head>
    <body>
        <div>
            <h1 class="bold">后记</h1>
            <p>TODO</p>
        </div>
    </body>
</html>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book2</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
        <item id="postscript.xhtml" href="Text/postscript.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
        <itemref idref="postscript.xhtml" />
    </spine>
    <guide />
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book2</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
        <navPoint id="navPoint-3" playOrder="3">
            <navLabel>
                <text>后记</text>
            </navLabel>
            <content src="Text/postscript.xhtml" />
        </navPoint>
    </navMap>
</ncx>
)");

  ptr.reset();

  std::filesystem::remove_all("test book2");
}

TEST_CASE("generate postscript and cover", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book3");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});
  epub.set_generate_postscript(true);
  epub.set_generate_cover(true);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book3"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book3");

  REQUIRE(std::filesystem::exists(kepub::Epub::cover_path));
  REQUIRE(klib::read_file(kepub::Epub::cover_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>封面</title>
    </head>
    <body>
        <div class="cover">
            <img alt="" src="../Images/cover.jpg" />
        </div>
    </body>
</html>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book3</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
        <meta name="cover" content="cover.jpg" />
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="cover.jpg" href="Images/cover.jpg" media-type="image/jpeg" />
        <item id="cover.xhtml" href="Text/cover.xhtml" media-type="application/xhtml+xml" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
        <item id="postscript.xhtml" href="Text/postscript.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="cover.xhtml" />
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
        <itemref idref="postscript.xhtml" />
    </spine>
    <guide>
        <reference type="cover" title="Cover" href="Text/cover.xhtml" />
    </guide>
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book3</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>封面</text>
            </navLabel>
            <content src="Text/cover.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-3" playOrder="3">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
        <navPoint id="navPoint-4" playOrder="4">
            <navLabel>
                <text>后记</text>
            </navLabel>
            <content src="Text/postscript.xhtml" />
        </navPoint>
    </navMap>
</ncx>
)");

  ptr.reset();

  std::filesystem::remove_all("test book3");
}

TEST_CASE("generate postscript, cover, illustration and image", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book4");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});
  epub.set_generate_postscript(true);
  epub.set_generate_cover(true);
  epub.set_illustration_num(3);
  epub.set_image_num(6);

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book4"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book4");

  for (std::int32_t i = 1; i <= 3; ++i) {
    auto file_name = "illustration00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    REQUIRE(std::filesystem::exists(path));
    REQUIRE(klib::read_file(path, false) ==
            fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>彩页00{}</title>
    </head>
    <body>
        <div>
            <div class="center">
                <img alt="00{}" src="../Images/00{}.jpg" />
            </div>
        </div>
    </body>
</html>
)",
                        std::to_string(i), std::to_string(i),
                        std::to_string(i)));
  }

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book4</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
        <meta name="cover" content="cover.jpg" />
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="x001.jpg" href="Images/001.jpg" media-type="image/jpeg" />
        <item id="x002.jpg" href="Images/002.jpg" media-type="image/jpeg" />
        <item id="x003.jpg" href="Images/003.jpg" media-type="image/jpeg" />
        <item id="x004.jpg" href="Images/004.jpg" media-type="image/jpeg" />
        <item id="x005.jpg" href="Images/005.jpg" media-type="image/jpeg" />
        <item id="x006.jpg" href="Images/006.jpg" media-type="image/jpeg" />
        <item id="cover.jpg" href="Images/cover.jpg" media-type="image/jpeg" />
        <item id="cover.xhtml" href="Text/cover.xhtml" media-type="application/xhtml+xml" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration001.xhtml" href="Text/illustration001.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration002.xhtml" href="Text/illustration002.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration003.xhtml" href="Text/illustration003.xhtml" media-type="application/xhtml+xml" />
        <item id="postscript.xhtml" href="Text/postscript.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="cover.xhtml" />
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
        <itemref idref="illustration001.xhtml" />
        <itemref idref="illustration002.xhtml" />
        <itemref idref="illustration003.xhtml" />
        <itemref idref="postscript.xhtml" />
    </spine>
    <guide>
        <reference type="cover" title="Cover" href="Text/cover.xhtml" />
    </guide>
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book4</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>封面</text>
            </navLabel>
            <content src="Text/cover.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-3" playOrder="3">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
        <navPoint id="navPoint-4" playOrder="4">
            <navLabel>
                <text>彩页</text>
            </navLabel>
            <content src="Text/illustration001.xhtml" />
        </navPoint>
        <navPoint id="navPoint-5" playOrder="5">
            <navLabel>
                <text>后记</text>
            </navLabel>
            <content src="Text/postscript.xhtml" />
        </navPoint>
    </navMap>
</ncx>
)");

  ptr.reset();

  std::filesystem::remove_all("test book4");
}

TEST_CASE("full generate", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book5");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});
  epub.set_generate_postscript(true);
  epub.set_generate_cover(true);
  epub.set_illustration_num(3);
  epub.set_image_num(6);

  epub.add_content("title 1", {"abc 1"});
  epub.add_content("title 2", {"abc 2"});
  epub.add_content("title 3", {"abc 3"});

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book5"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book5");

  for (std::int32_t i = 1; i <= 3; ++i) {
    auto file_name = "chapter00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    REQUIRE(std::filesystem::exists(path));
    REQUIRE(klib::read_file(path, false) ==
            fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>title {}</title>
    </head>
    <body>
        <div>
            <h1 class="bold">title {}</h1>
            <p>abc {}</p>
        </div>
    </body>
</html>
)",
                        std::to_string(i), std::to_string(i),
                        std::to_string(i)));
  }

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book5</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
        <meta name="cover" content="cover.jpg" />
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="x001.jpg" href="Images/001.jpg" media-type="image/jpeg" />
        <item id="x002.jpg" href="Images/002.jpg" media-type="image/jpeg" />
        <item id="x003.jpg" href="Images/003.jpg" media-type="image/jpeg" />
        <item id="x004.jpg" href="Images/004.jpg" media-type="image/jpeg" />
        <item id="x005.jpg" href="Images/005.jpg" media-type="image/jpeg" />
        <item id="x006.jpg" href="Images/006.jpg" media-type="image/jpeg" />
        <item id="cover.jpg" href="Images/cover.jpg" media-type="image/jpeg" />
        <item id="cover.xhtml" href="Text/cover.xhtml" media-type="application/xhtml+xml" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration001.xhtml" href="Text/illustration001.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration002.xhtml" href="Text/illustration002.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration003.xhtml" href="Text/illustration003.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter001.xhtml" href="Text/chapter001.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter002.xhtml" href="Text/chapter002.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter003.xhtml" href="Text/chapter003.xhtml" media-type="application/xhtml+xml" />
        <item id="postscript.xhtml" href="Text/postscript.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="cover.xhtml" />
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
        <itemref idref="illustration001.xhtml" />
        <itemref idref="illustration002.xhtml" />
        <itemref idref="illustration003.xhtml" />
        <itemref idref="chapter001.xhtml" />
        <itemref idref="chapter002.xhtml" />
        <itemref idref="chapter003.xhtml" />
        <itemref idref="postscript.xhtml" />
    </spine>
    <guide>
        <reference type="cover" title="Cover" href="Text/cover.xhtml" />
    </guide>
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book5</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>封面</text>
            </navLabel>
            <content src="Text/cover.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-3" playOrder="3">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
        <navPoint id="navPoint-4" playOrder="4">
            <navLabel>
                <text>彩页</text>
            </navLabel>
            <content src="Text/illustration001.xhtml" />
        </navPoint>
        <navPoint id="navPoint-5" playOrder="5">
            <navLabel>
                <text>title 1</text>
            </navLabel>
            <content src="Text/chapter001.xhtml" />
        </navPoint>
        <navPoint id="navPoint-6" playOrder="6">
            <navLabel>
                <text>title 2</text>
            </navLabel>
            <content src="Text/chapter002.xhtml" />
        </navPoint>
        <navPoint id="navPoint-7" playOrder="7">
            <navLabel>
                <text>title 3</text>
            </navLabel>
            <content src="Text/chapter003.xhtml" />
        </navPoint>
        <navPoint id="navPoint-8" playOrder="8">
            <navLabel>
                <text>后记</text>
            </navLabel>
            <content src="Text/postscript.xhtml" />
        </navPoint>
    </navMap>
</ncx>
)");

  ptr.reset();

  std::filesystem::remove_all("test book5");
}

TEST_CASE("sub-volume", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book6");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});
  epub.set_generate_postscript(true);
  epub.set_generate_cover(true);
  epub.set_illustration_num(3);
  epub.set_image_num(6);

  epub.add_content("volume 1", "title 1", {"abc 1"});
  epub.add_content("volume 1", "title 2", {"abc 2"});
  epub.add_content("volume 2", "title 3", {"abc 3"});

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book6"));

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book6");

  for (std::int32_t i = 1; i <= 3; ++i) {
    auto file_name = "chapter00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    REQUIRE(std::filesystem::exists(path));
    REQUIRE(klib::read_file(path, false) ==
            fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>title {}</title>
    </head>
    <body>
        <div>
            <h1 class="bold">title {}</h1>
            <p>abc {}</p>
        </div>
    </body>
</html>
)",
                        std::to_string(i), std::to_string(i),
                        std::to_string(i)));
  }

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book6</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
        <meta name="cover" content="cover.jpg" />
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="x001.jpg" href="Images/001.jpg" media-type="image/jpeg" />
        <item id="x002.jpg" href="Images/002.jpg" media-type="image/jpeg" />
        <item id="x003.jpg" href="Images/003.jpg" media-type="image/jpeg" />
        <item id="x004.jpg" href="Images/004.jpg" media-type="image/jpeg" />
        <item id="x005.jpg" href="Images/005.jpg" media-type="image/jpeg" />
        <item id="x006.jpg" href="Images/006.jpg" media-type="image/jpeg" />
        <item id="cover.jpg" href="Images/cover.jpg" media-type="image/jpeg" />
        <item id="cover.xhtml" href="Text/cover.xhtml" media-type="application/xhtml+xml" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration001.xhtml" href="Text/illustration001.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration002.xhtml" href="Text/illustration002.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration003.xhtml" href="Text/illustration003.xhtml" media-type="application/xhtml+xml" />
        <item id="volume001.xhtml" href="Text/volume001.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter001.xhtml" href="Text/chapter001.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter002.xhtml" href="Text/chapter002.xhtml" media-type="application/xhtml+xml" />
        <item id="volume002.xhtml" href="Text/volume002.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter003.xhtml" href="Text/chapter003.xhtml" media-type="application/xhtml+xml" />
        <item id="postscript.xhtml" href="Text/postscript.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="cover.xhtml" />
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
        <itemref idref="illustration001.xhtml" />
        <itemref idref="illustration002.xhtml" />
        <itemref idref="illustration003.xhtml" />
        <itemref idref="volume001.xhtml" />
        <itemref idref="chapter001.xhtml" />
        <itemref idref="chapter002.xhtml" />
        <itemref idref="volume002.xhtml" />
        <itemref idref="chapter003.xhtml" />
        <itemref idref="postscript.xhtml" />
    </spine>
    <guide>
        <reference type="cover" title="Cover" href="Text/cover.xhtml" />
    </guide>
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book6</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>封面</text>
            </navLabel>
            <content src="Text/cover.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-3" playOrder="3">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
        <navPoint id="navPoint-4" playOrder="4">
            <navLabel>
                <text>彩页</text>
            </navLabel>
            <content src="Text/illustration001.xhtml" />
        </navPoint>
        <navPoint id="navPoint-5" playOrder="5">
            <navLabel>
                <text>volume 1</text>
            </navLabel>
            <content src="Text/volume001.xhtml" />
            <navPoint id="navPoint-6" playOrder="6">
                <navLabel>
                    <text>title 1</text>
                </navLabel>
                <content src="Text/chapter001.xhtml" />
            </navPoint>
            <navPoint id="navPoint-7" playOrder="7">
                <navLabel>
                    <text>title 2</text>
                </navLabel>
                <content src="Text/chapter002.xhtml" />
            </navPoint>
        </navPoint>
        <navPoint id="navPoint-8" playOrder="8">
            <navLabel>
                <text>volume 2</text>
            </navLabel>
            <content src="Text/volume002.xhtml" />
            <navPoint id="navPoint-9" playOrder="9">
                <navLabel>
                    <text>title 3</text>
                </navLabel>
                <content src="Text/chapter003.xhtml" />
            </navPoint>
        </navPoint>
        <navPoint id="navPoint-10" playOrder="10">
            <navLabel>
                <text>后记</text>
            </navLabel>
            <content src="Text/postscript.xhtml" />
        </navPoint>
    </navMap>
</ncx>
)");

  ptr.reset();

  std::filesystem::remove_all("test book6");
}

TEST_CASE("append", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book7");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});
  epub.set_generate_postscript(true);
  epub.set_generate_cover(true);
  epub.set_illustration_num(3);
  epub.set_image_num(6);

  epub.add_content("title 1", {"abc 1"});
  epub.add_content("title 2", {"abc 2"});
  epub.add_content("title 3", {"abc 3"});

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book7"));

  kepub::Epub::append_chapter(
      "test book7", {{"", "title 4", {"abc 4"}}, {"", "title 5", {"abc 5"}}});

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book7");

  for (std::int32_t i = 1; i <= 5; ++i) {
    auto file_name = "chapter00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    REQUIRE(std::filesystem::exists(path));
    REQUIRE(klib::read_file(path, false) ==
            fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>title {}</title>
    </head>
    <body>
        <div>
            <h1 class="bold">title {}</h1>
            <p>abc {}</p>
        </div>
    </body>
</html>
)",
                        std::to_string(i), std::to_string(i),
                        std::to_string(i)));
  }

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book7</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
        <meta name="cover" content="cover.jpg" />
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="x001.jpg" href="Images/001.jpg" media-type="image/jpeg" />
        <item id="x002.jpg" href="Images/002.jpg" media-type="image/jpeg" />
        <item id="x003.jpg" href="Images/003.jpg" media-type="image/jpeg" />
        <item id="x004.jpg" href="Images/004.jpg" media-type="image/jpeg" />
        <item id="x005.jpg" href="Images/005.jpg" media-type="image/jpeg" />
        <item id="x006.jpg" href="Images/006.jpg" media-type="image/jpeg" />
        <item id="cover.jpg" href="Images/cover.jpg" media-type="image/jpeg" />
        <item id="cover.xhtml" href="Text/cover.xhtml" media-type="application/xhtml+xml" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration001.xhtml" href="Text/illustration001.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration002.xhtml" href="Text/illustration002.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration003.xhtml" href="Text/illustration003.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter001.xhtml" href="Text/chapter001.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter002.xhtml" href="Text/chapter002.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter003.xhtml" href="Text/chapter003.xhtml" media-type="application/xhtml+xml" />
        <item id="postscript.xhtml" href="Text/postscript.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter004.xhtml" href="Text/chapter004.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter005.xhtml" href="Text/chapter005.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="cover.xhtml" />
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
        <itemref idref="illustration001.xhtml" />
        <itemref idref="illustration002.xhtml" />
        <itemref idref="illustration003.xhtml" />
        <itemref idref="chapter001.xhtml" />
        <itemref idref="chapter002.xhtml" />
        <itemref idref="chapter003.xhtml" />
        <itemref idref="postscript.xhtml" />
        <itemref idref="chapter004.xhtml" />
        <itemref idref="chapter005.xhtml" />
    </spine>
    <guide>
        <reference type="cover" title="Cover" href="Text/cover.xhtml" />
    </guide>
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book7</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>封面</text>
            </navLabel>
            <content src="Text/cover.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-3" playOrder="3">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
        <navPoint id="navPoint-4" playOrder="4">
            <navLabel>
                <text>彩页</text>
            </navLabel>
            <content src="Text/illustration001.xhtml" />
        </navPoint>
        <navPoint id="navPoint-5" playOrder="5">
            <navLabel>
                <text>title 1</text>
            </navLabel>
            <content src="Text/chapter001.xhtml" />
        </navPoint>
        <navPoint id="navPoint-6" playOrder="6">
            <navLabel>
                <text>title 2</text>
            </navLabel>
            <content src="Text/chapter002.xhtml" />
        </navPoint>
        <navPoint id="navPoint-7" playOrder="7">
            <navLabel>
                <text>title 3</text>
            </navLabel>
            <content src="Text/chapter003.xhtml" />
        </navPoint>
        <navPoint id="navPoint-8" playOrder="8">
            <navLabel>
                <text>后记</text>
            </navLabel>
            <content src="Text/postscript.xhtml" />
        </navPoint>
        <navPoint id="navPoint-9" playOrder="9">
            <navLabel>
                <text>title 4</text>
            </navLabel>
            <content src="Text/chapter004.xhtml" />
        </navPoint>
        <navPoint id="navPoint-10" playOrder="10">
            <navLabel>
                <text>title 5</text>
            </navLabel>
            <content src="Text/chapter005.xhtml" />
        </navPoint>
    </navMap>
</ncx>
)");

  ptr.reset();

  std::filesystem::remove_all("test book7");
}

TEST_CASE("append sub-volume", "[epub]") {
  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name("test book8");
  epub.set_author("test author");
  epub.set_introduction({"test", "introduction"});
  epub.set_generate_cover(true);
  epub.set_illustration_num(3);
  epub.set_image_num(6);

  epub.add_content("volume 1", "title 1", {"abc 1"});
  epub.add_content("volume 1", "title 2", {"abc 2"});
  epub.add_content("volume 2", "title 3", {"abc 3"});

  epub.set_uuid("5208e6bb-5d25-45b0-a7fd-b97d79a85fd4");
  epub.set_date("2021-08-01");

  REQUIRE_NOTHROW(epub.generate());
  REQUIRE(std::filesystem::is_directory("test book8"));

  kepub::Epub::append_chapter("test book8",
                              {{"", "title 4", {"abc 4"}},
                               {"volume 3", "title 5", {"abc 5"}},
                               {"volume 3", "title 6", {"abc 6"}}});

  auto ptr = std::make_unique<klib::ChangeWorkingDir>("test book8");

  for (std::int32_t i = 1; i <= 6; ++i) {
    auto file_name = "chapter00" + std::to_string(i) + ".xhtml";
    auto path = std::filesystem::path(kepub::Epub::text_dir) / file_name;

    REQUIRE(std::filesystem::exists(path));
    REQUIRE(klib::read_file(path, false) ==
            fmt::format(R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="zh-CN" xmlns:epub="http://www.idpf.org/2007/ops">
    <head>
        <link href="../Styles/style.css" rel="stylesheet" type="text/css" />
        <title>title {}</title>
    </head>
    <body>
        <div>
            <h1 class="bold">title {}</h1>
            <p>abc {}</p>
        </div>
    </body>
</html>
)",
                        std::to_string(i), std::to_string(i),
                        std::to_string(i)));
  }

  REQUIRE(std::filesystem::exists(kepub::Epub::content_path));
  REQUIRE(klib::read_file(kepub::Epub::content_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<package version="2.0" unique-identifier="BookId" xmlns="http://www.idpf.org/2007/opf">
    <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>test book8</dc:title>
        <dc:creator opf:file-as="kaiser" opf:role="aut">test author</dc:creator>
        <dc:language>zh-CN</dc:language>
        <dc:rights>kaiser</dc:rights>
        <dc:date opf:event="modification" xmlns:opf="http://www.idpf.org/2007/opf">2021-08-01</dc:date>
        <dc:identifier id="BookId" opf:scheme="UUID">urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4</dc:identifier>
        <meta name="cover" content="cover.jpg" />
    </metadata>
    <manifest>
        <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml" />
        <item id="style.css" href="Styles/style.css" media-type="text/css" />
        <item id="SourceHanSansSC-Bold.otf" href="Fonts/SourceHanSansSC-Bold.otf" media-type="application/vnd.ms-opentype" />
        <item id="x001.jpg" href="Images/001.jpg" media-type="image/jpeg" />
        <item id="x002.jpg" href="Images/002.jpg" media-type="image/jpeg" />
        <item id="x003.jpg" href="Images/003.jpg" media-type="image/jpeg" />
        <item id="x004.jpg" href="Images/004.jpg" media-type="image/jpeg" />
        <item id="x005.jpg" href="Images/005.jpg" media-type="image/jpeg" />
        <item id="x006.jpg" href="Images/006.jpg" media-type="image/jpeg" />
        <item id="cover.jpg" href="Images/cover.jpg" media-type="image/jpeg" />
        <item id="cover.xhtml" href="Text/cover.xhtml" media-type="application/xhtml+xml" />
        <item id="message.xhtml" href="Text/message.xhtml" media-type="application/xhtml+xml" />
        <item id="introduction.xhtml" href="Text/introduction.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration001.xhtml" href="Text/illustration001.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration002.xhtml" href="Text/illustration002.xhtml" media-type="application/xhtml+xml" />
        <item id="illustration003.xhtml" href="Text/illustration003.xhtml" media-type="application/xhtml+xml" />
        <item id="volume001.xhtml" href="Text/volume001.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter001.xhtml" href="Text/chapter001.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter002.xhtml" href="Text/chapter002.xhtml" media-type="application/xhtml+xml" />
        <item id="volume002.xhtml" href="Text/volume002.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter003.xhtml" href="Text/chapter003.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter004.xhtml" href="Text/chapter004.xhtml" media-type="application/xhtml+xml" />
        <item id="volume003.xhtml" href="Text/volume003.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter005.xhtml" href="Text/chapter005.xhtml" media-type="application/xhtml+xml" />
        <item id="chapter006.xhtml" href="Text/chapter006.xhtml" media-type="application/xhtml+xml" />
    </manifest>
    <spine toc="ncx">
        <itemref idref="cover.xhtml" />
        <itemref idref="message.xhtml" />
        <itemref idref="introduction.xhtml" />
        <itemref idref="illustration001.xhtml" />
        <itemref idref="illustration002.xhtml" />
        <itemref idref="illustration003.xhtml" />
        <itemref idref="volume001.xhtml" />
        <itemref idref="chapter001.xhtml" />
        <itemref idref="chapter002.xhtml" />
        <itemref idref="volume002.xhtml" />
        <itemref idref="chapter003.xhtml" />
        <itemref idref="chapter004.xhtml" />
        <itemref idref="volume003.xhtml" />
        <itemref idref="chapter005.xhtml" />
        <itemref idref="chapter006.xhtml" />
    </spine>
    <guide>
        <reference type="cover" title="Cover" href="Text/cover.xhtml" />
    </guide>
</package>
)");

  REQUIRE(std::filesystem::exists(kepub::Epub::toc_path));
  REQUIRE(klib::read_file(kepub::Epub::toc_path, false) ==
          R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
<ncx version="2005-1" xmlns="http://www.daisy.org/z3986/2005/ncx/">
    <head>
        <meta name="dtb:uid" content="urn:uuid:5208e6bb-5d25-45b0-a7fd-b97d79a85fd4" />
        <meta name="dtb:depth" content="1" />
        <meta name="dtb:totalPageCount" content="0" />
        <meta name="dtb:maxPageNumber" content="0" />
    </head>
    <docTitle>
        <text>test book8</text>
    </docTitle>
    <docAuthor>
        <text>test author</text>
    </docAuthor>
    <navMap>
        <navPoint id="navPoint-1" playOrder="1">
            <navLabel>
                <text>封面</text>
            </navLabel>
            <content src="Text/cover.xhtml" />
        </navPoint>
        <navPoint id="navPoint-2" playOrder="2">
            <navLabel>
                <text>制作信息</text>
            </navLabel>
            <content src="Text/message.xhtml" />
        </navPoint>
        <navPoint id="navPoint-3" playOrder="3">
            <navLabel>
                <text>简介</text>
            </navLabel>
            <content src="Text/introduction.xhtml" />
        </navPoint>
        <navPoint id="navPoint-4" playOrder="4">
            <navLabel>
                <text>彩页</text>
            </navLabel>
            <content src="Text/illustration001.xhtml" />
        </navPoint>
        <navPoint id="navPoint-5" playOrder="5">
            <navLabel>
                <text>volume 1</text>
            </navLabel>
            <content src="Text/volume001.xhtml" />
            <navPoint id="navPoint-6" playOrder="6">
                <navLabel>
                    <text>title 1</text>
                </navLabel>
                <content src="Text/chapter001.xhtml" />
            </navPoint>
            <navPoint id="navPoint-7" playOrder="7">
                <navLabel>
                    <text>title 2</text>
                </navLabel>
                <content src="Text/chapter002.xhtml" />
            </navPoint>
        </navPoint>
        <navPoint id="navPoint-8" playOrder="8">
            <navLabel>
                <text>volume 2</text>
            </navLabel>
            <content src="Text/volume002.xhtml" />
            <navPoint id="navPoint-9" playOrder="9">
                <navLabel>
                    <text>title 3</text>
                </navLabel>
                <content src="Text/chapter003.xhtml" />
            </navPoint>
            <navPoint id="navPoint-10" playOrder="10">
                <navLabel>
                    <text>title 4</text>
                </navLabel>
                <content src="Text/chapter004.xhtml" />
            </navPoint>
        </navPoint>
        <navPoint id="navPoint-11" playOrder="11">
            <navLabel>
                <text>volume 3</text>
            </navLabel>
            <content src="Text/volume003.xhtml" />
            <navPoint id="navPoint-12" playOrder="12">
                <navLabel>
                    <text>title 5</text>
                </navLabel>
                <content src="Text/chapter005.xhtml" />
            </navPoint>
            <navPoint id="navPoint-13" playOrder="13">
                <navLabel>
                    <text>title 6</text>
                </navLabel>
                <content src="Text/chapter006.xhtml" />
            </navPoint>
        </navPoint>
    </navMap>
</ncx>
)");

  ptr.reset();

  std::filesystem::remove_all("test book8");
}
