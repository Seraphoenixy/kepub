#include "util.h"

int main(int argc, const char *argv[]) {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  auto login_name = kepub::get_login_name();
  auto password = kepub::get_password();
}
