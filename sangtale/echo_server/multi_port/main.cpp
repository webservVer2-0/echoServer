#include "Server.hpp"
#include "syscall.hpp"

int main(int argc, char **av) {
  Server server(argc, av);

  server.bind_socket();
  server.listen_and_fcntl();
  server.event_controller();
  return (EXIT_SUCCESS);
}