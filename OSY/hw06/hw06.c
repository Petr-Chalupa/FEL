#define FD_STDIN 0
#define FD_STDOUT 1
#define FD_STDERR 2

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4

int isnum(char ch);
int isspc(char ch);
int _strlen(char *buf);
void _strrev(char *buf);
void _sprinthex(char *buf, unsigned num);
static void print(unsigned num);
static int sys_read(int fd, void *buf, unsigned count);
static int sys_write(int fd, void *buf, unsigned count);
static void sys_exit(int code);

int main() {
  char buf[20];
  unsigned num = 0;
  int i = 0;
  int num_digits = 0;
  unsigned chars_to_process = 0;

  for (/* no init */; /* no end condition */; i++, chars_to_process--) {
    if (chars_to_process == 0) {
      int ret = sys_read(FD_STDIN, buf, sizeof(buf));
      if (ret < 0) sys_exit(1);
      i = 0;
      chars_to_process = ret;
    }
    if (num_digits > 0 && (chars_to_process == 0 /* EOF */ || !isnum(buf[i]))) {
      print(num);
      num_digits = 0;
      num = 0;
    }
    if (chars_to_process == 0 /* EOF */ || (!isspc(buf[i]) && !isnum(buf[i]))) {
      sys_exit(0);
    }
    if (isnum(buf[i])) {
      num = num * 10 + buf[i] - '0';
      num_digits++;
    }
  }
}

void _start() {
  int ret = main();
  sys_exit(ret);
}

int isnum(char ch) { return ch >= '0' && ch <= '9'; }

int isspc(char ch) { return ch == ' ' || ch == '\n'; }

int _strlen(char *buf) {
  int count = 0;

  while (buf[count] != '\0') {
    count++;
  }

  return count;
}

void _strrev(char *buf) {
  int l = 0;
  int r = _strlen(buf) - 1; // -1 -> don't swap \0
  char temp;

  while (l < r) {
    temp = buf[l];
    buf[l] = buf[r];
    buf[r] = temp;

    l++;
    r--;
  }
}

void _sprinthex(char *buf, unsigned num) {
  int i = 0, temp;

  buf[i++] = '\n';

  if (num == 0) {
    buf[i++] = '0';
  }

  while (num != 0) {
    temp = num % 16;
    temp = temp < 10 ? temp + '0' : temp - 10 + 'a';

    buf[i++] = temp;
    num = num / 16;
  }

  buf[i++] = 'x';
  buf[i++] = '0';
  buf[i] = '\0';

  _strrev(buf);
}

static void print(unsigned num) {
  char buf[20];
  _sprinthex(buf, num);

  int ret = sys_write(FD_STDOUT, buf, _strlen(buf));
  if (ret < 0) sys_exit(1);
}

static int sys_read(int fd, void *buf, unsigned count) {
  int ret;

  asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_READ), "b"(fd), "c"(buf), "d"(count) : "memory");

  return ret;
}

static int sys_write(int fd, void *buf, unsigned count) {
  int ret;

  asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_WRITE), "b"(fd), "c"(buf), "d"(count) : "memory");

  return ret;
}

static void sys_exit(int code) {
  asm volatile("int $0x80" : : "a"(SYS_EXIT), "b"(code) : "memory");
  __builtin_unreachable();
}
