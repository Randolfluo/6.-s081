#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

int 
ParseLine(char *buf) {

  int i = 0;
  while ((read(0, buf+i, sizeof(char))) > 0) {
      if(buf[i] == '\0' || buf[i] == '\n')
      {
          buf[i] = '\0';
          return 1;
      }
      i++;
  }
  if(i != 0)
  {
      buf[i] = '\0';
      return 1;
  }
  return 0;
}

int 
main(int argc, char *argv[]) {
  char *line[MAXARG];
  char buf[MAXARG];

  for (int i = 1; i < argc; i++) {
    line[i - 1] = argv[i];
  }
  while (ParseLine(buf)) {
    if (fork() == 0) {
      line[argc - 1] = buf;
      line[argc] = 0;
      exec(argv[1], line);
      exit(0);
    } else {
      wait(0);
    }
  }

  exit(0);
}
