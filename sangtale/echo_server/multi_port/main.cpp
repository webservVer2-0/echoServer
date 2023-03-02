#include "Server.hpp"
#include "syscall.hpp"

int main(int argc, char **av) {
  Server server(argc, av);

  server.bind_socket();
  server.listen_and_fcntl();
  server.file_setting();
  server.print_server_info();
  server.event_controller();
  return (EXIT_SUCCESS);
}