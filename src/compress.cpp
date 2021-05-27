#include "compress.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>

#include <archive.h>
#include <archive_entry.h>

#include "error.h"
#include "util.h"

namespace {

void copy_data(struct archive *ar, struct archive *aw) {
  while (true) {
    const void *buff;
    std::size_t size;
    la_int64_t offset;

    la_ssize_t status = archive_read_data_block(ar, &buff, &size, &offset);
    if (status == ARCHIVE_EOF) {
      return;
    }
    if (status != ARCHIVE_OK) {
      kepub::error(archive_error_string(aw));
    }

    status = archive_write_data_block(aw, buff, size, offset);
    if (status != ARCHIVE_OK) {
      kepub::error(archive_error_string(aw));
    }
  }
}

}  // namespace

namespace kepub {

void compress(const std::string &dir) {
  auto archive = archive_write_new();

  archive_write_set_format_zip(archive);
  archive_write_add_filter_none(archive);

  auto out = dir + ".zip";
  auto status = archive_write_open_filename(archive, out.c_str());
  if (status != ARCHIVE_OK) {
    error(archive_error_string(archive));
  }

  if (!std::filesystem::is_directory(dir)) {
    error("{} is not directory", dir);
  }

  ChangeWorkDir change_work_dir(dir);
  for (const auto &item :
       std::filesystem::directory_iterator(std::filesystem::current_path())) {
    auto disk = archive_read_disk_new();
    archive_read_disk_set_standard_lookup(disk);
    status = archive_read_disk_open(disk, item.path().filename().c_str());
    if (status != ARCHIVE_OK) {
      error(archive_error_string(disk));
    }

    while (true) {
      auto entry = archive_entry_new();
      status = archive_read_next_header2(disk, entry);
      if (status == ARCHIVE_EOF) {
        archive_entry_free(entry);
        break;
      }
      if (status != ARCHIVE_OK) {
        error(archive_error_string(disk));
      }

      archive_read_disk_descend(disk);

      status = archive_write_header(archive, entry);
      if (status != ARCHIVE_OK) {
        error(archive_error_string(disk));
      }

      char buff[16384];
      auto file = std::fopen(archive_entry_sourcepath(entry), "rb");
      if (!file) {
        error("open file error");
      }

      auto len = std::fread(buff, 1, sizeof(buff), file);
      while (len > 0) {
        archive_write_data(archive, buff, static_cast<std::size_t>(len));
        len = std::fread(buff, 1, sizeof(buff), file);
      }

      std::fclose(file);
      archive_entry_free(entry);
    }

    archive_read_close(disk);
    archive_read_free(disk);
  }

  archive_write_close(archive);
  archive_write_free(archive);
}

void decompress(const std::string &file_name, const std::string &dir) {
  std::int32_t flags = (ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                        ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);

  auto archive = archive_read_new();
  archive_read_support_format_zip(archive);

  auto extract = archive_write_disk_new();
  archive_write_disk_set_options(extract, flags);
  archive_write_disk_set_standard_lookup(extract);

  std::int32_t status =
      archive_read_open_filename(archive, file_name.c_str(), 10240);
  if (status != ARCHIVE_OK) {
    error("archive_read_open_filename error");
  }

  if (!std::empty(dir)) {
    create_dir(dir);
  }
  ChangeWorkDir change_work_dir(dir);

  while (true) {
    struct archive_entry *entry;
    status = archive_read_next_header(archive, &entry);
    if (status == ARCHIVE_EOF) {
      break;
    }
    if (status != ARCHIVE_OK) {
      error(archive_error_string(archive));
    }

    status = archive_write_header(extract, entry);
    if (status != ARCHIVE_OK) {
      error(archive_error_string(archive));
    }

    if (archive_entry_size(entry) > 0) {
      copy_data(archive, extract);
    }

    status = archive_write_finish_entry(extract);
    if (status != ARCHIVE_OK) {
      error(archive_error_string(archive));
    }
  }

  archive_read_close(archive);
  archive_read_free(archive);
  archive_write_close(extract);
  archive_write_free(extract);
}

}  // namespace kepub
