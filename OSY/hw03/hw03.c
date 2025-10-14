#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define EXEC_TIME_S 5
#define GEN_INTERVAL_S 1
#define GEN_RAND_LIMIT 4096

typedef enum { ERR_CHILD_PROCESS_FAILURE = 1, ERR_SYSCALL_FAILURE = 2 } Error;

typedef struct {
  int fd[2];
} Pipe;

void Pipe__close(Pipe *p);
void GEN__handle_sigterm();
int GEN(Pipe *p);
int NSD(Pipe *p);

int main() {
  // --- Create pipe ---
  Pipe p = {0};

  if (pipe(p.fd) != 0) {
    exit(ERR_SYSCALL_FAILURE);
  }

  // --- Create GEN child process ---
  pid_t gen_pid = fork();

  if (gen_pid == -1) {
    Pipe__close(&p);
    exit(ERR_SYSCALL_FAILURE);
  } else if (gen_pid == 0) {
    GEN(&p);
  }

  // --- Create NSD child process ---
  pid_t nsd_pid = fork();

  if (nsd_pid == -1) {
    Pipe__close(&p);
    kill(gen_pid, SIGTERM);
    exit(ERR_SYSCALL_FAILURE);
  } else if (nsd_pid == 0) {
    NSD(&p);
  }

  // --- Parent wait for exec and then kill GEN ---
  Pipe__close(&p);

  sleep(EXEC_TIME_S);
  kill(gen_pid, SIGTERM);

  int status_1, status_2;
  wait(&status_1);
  wait(&status_2);

  int ret_1 = WIFEXITED(status_1) ? WEXITSTATUS(status_1) : ERR_CHILD_PROCESS_FAILURE;
  int ret_2 = WIFEXITED(status_2) ? WEXITSTATUS(status_2) : ERR_CHILD_PROCESS_FAILURE;
  int ret = ret_1 || ret_2;
  printf("%s\n", ret ? "ERROR" : "OK");

  return ret;
}

void Pipe__close(Pipe *p) {
  close(p->fd[0]);
  close(p->fd[1]);
}

void GEN__handle_sigterm() {
  // Signal safe message writing
  const char msg[] = "GEN TERMINATED\n";
  write(STDERR_FILENO, msg, sizeof(msg) - 1);

  // Treat as OK - expected from parent process
  _exit(0);
}

int GEN(Pipe *p) {
  // --- Handle SIGTERM ---
  if (signal(SIGTERM, GEN__handle_sigterm) == SIG_ERR) {
    Pipe__close(p);
    _exit(ERR_SYSCALL_FAILURE);
  }

  // --- Connect to the pipe as writer ---
  int dup_r = dup2(p->fd[1], STDOUT_FILENO);
  Pipe__close(p);

  if (dup_r == -1) {
    _exit(ERR_SYSCALL_FAILURE);
  }

  // --- Print random numbers ---
  while (1) {
    printf("%d %d\n", rand() % GEN_RAND_LIMIT, rand() % GEN_RAND_LIMIT);
    fflush(stdout);
    sleep(GEN_INTERVAL_S);
  }

  _exit(0);
}

int NSD(Pipe *p) {
  // --- Connect to the pipe as reader ---
  int dup_r = dup2(p->fd[0], STDIN_FILENO);
  Pipe__close(p);

  if (dup_r == -1) {
    _exit(ERR_SYSCALL_FAILURE);
  }

  // --- Execute nsd ---
  execl("./nsd", "nsd", (char *)NULL);

  _exit(ERR_SYSCALL_FAILURE); // Executed only if execl fails
}