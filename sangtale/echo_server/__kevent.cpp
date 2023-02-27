#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  struct kevent event;
  int kq, nevent;

  /* Create a new kernel event queue */
  kq = kqueue();
  if (kq == -1) {
    perror("kqueue");
    exit(1);
  }
  /* Register the standard input file descriptor with the kqueue */
  EV_SET(&event, STDIN_FILENO, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
  if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
    perror("kevent() error");
    exit(1);
  }

  /* Wait for an event to occur */
  while (1) {
    nevent = kevent(kq, NULL, 0, &event, 1, NULL);
    if (nevent == -1) {
      perror("kevent() error");
      exit(1);
    }
    if (nevent > 0) {
      printf("Event Count : %d\n", nevent);
      printf("Event occurred on file descriptor %lu\n", event.ident);
      printf("Filter: %d\n", event.filter);
      printf("Flags: %d\n", event.flags);
      printf("FFlags: %d\n", event.fflags);
      printf("Data: %ld\n", event.data);
      printf("Udata: %p\n", event.udata);
      break;
    }
  }
  return (0);
}