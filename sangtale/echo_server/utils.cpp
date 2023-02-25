#include "echo_server.hpp"

void exit_with_error(const std::string &msg) {
  std::cerr << msg << "\n";
  exit(EXIT_FAILURE);
}
