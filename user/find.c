#include "kernel/fs.h"
#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

char *fmtname(char *path) {
  static char buf[DIRSIZ + 1]; // static防止返回时被清理
  char *p;
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p)); //防止字符串被覆盖
  buf[strlen(p)] = 0;         // 确保以零结尾
  return buf;
}

int match(char *str1, char *str2) { return strcmp(str1, str2) == 0; }

void find(char *path, char *str) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_FILE:
    if (match(fmtname(path), str))
      printf("%s\n", path);
    break;
  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {

      if (de.inum == 0 || de.inum == 1 || match(de.name, ".") ||
          match(de.name, ".."))
        continue;
      memmove(p, de.name, strlen(de.name));
      p[strlen(de.name)] = 0;
      find(buf, str);
    }
    break;
  }

  close(fd);
}

int main(int argc, char *argv[]) {
  // int i;
  if (argc != 3) {
    char str[15] = "Args must be 3!";
    write(2, str, strlen(str));
    exit(1);
  }

  find(argv[1], argv[2]);
  exit(0);
}
