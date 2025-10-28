#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum { WORKING = -1, DONE = 0, ERROR = 1 } ProducerState;

typedef struct Node {
  int x;
  char *word;
  struct Node *next;
} Node;

typedef struct {
  Node *head;
  Node *tail;
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  ProducerState producer_state;
} List;

static List list;
static pthread_mutex_t stdout_mtx = PTHREAD_MUTEX_INITIALIZER;

Node *Node__init();
void Node__free(Node *n);
void List__init(List *l);
void List__destroy(List *l);
void *producer(void *arg);
void *consumer(void *arg);

int main(int argc, char *argv[]) {
  // --- Initialize list ---
  List__init(&list);

  // --- Get number of consumers to spawn ---
  int N = argc > 1 ? atoi(argv[1]) : 1;

  if (N < 1 || N > sysconf(_SC_NPROCESSORS_ONLN)) {
    return 1;
  }

  // --- Spawn threads ---
  pthread_t producer_thread;
  pthread_t consumers[N];

  if (pthread_create(&producer_thread, NULL, producer, NULL) != 0) {
    return 1;
  }

  for (long i = 0; i < N; i++) {
    if (pthread_create(&consumers[i], NULL, consumer, (void *)(i + 1)) != 0) {
      return 1;
    }
  }

  // --- Wait for threads ---
  pthread_join(producer_thread, NULL);

  for (int i = 0; i < N; i++) {
    pthread_join(consumers[i], NULL);
  }

  // --- Destroy list ---
  List__destroy(&list);

  return list.producer_state == ERROR ? 1 : 0;
}

Node *Node__init() {
  Node *n = malloc(sizeof(Node));
  if (n == NULL) return NULL;

  n->word = NULL;
  n->next = NULL;

  return n;
}

void Node__free(Node *n) {
  free(n->word);
  free(n);
}

void List__init(List *l) {
  l->head = NULL;
  l->tail = NULL;
  pthread_mutex_init(&l->mtx, NULL);
  pthread_cond_init(&l->cond, NULL);
  l->producer_state = WORKING;
}

void List__destroy(List *l) {
  // Empty list is expected
  pthread_mutex_destroy(&l->mtx);
  pthread_cond_destroy(&l->cond);
}

void List__push(List *l, Node *n) {
  pthread_mutex_lock(&l->mtx);

  if (l->tail == NULL) {
    l->head = l->tail = n;
  } else {
    l->tail->next = n;
    l->tail = n;
  }

  pthread_cond_signal(&l->cond);
  pthread_mutex_unlock(&l->mtx);
}

Node *List__pop(List *l) {
  pthread_mutex_lock(&l->mtx);

  while (l->head == NULL && l->producer_state == WORKING) {
    pthread_cond_wait(&l->cond, &l->mtx);
  }
  if (l->head == NULL && l->producer_state != WORKING) {
    pthread_mutex_unlock(&l->mtx);
    return NULL;
  }

  Node *n = l->head;
  l->head = n->next;
  if (l->head == NULL) l->tail = NULL;

  pthread_mutex_unlock(&l->mtx);

  return n;
}

void *producer(void *arg) {
  int ret, x;
  char *word;

  while ((ret = scanf("%d %ms", &x, &word)) == 2) {
    if (x < 0) {
      free(word);
      pthread_mutex_lock(&list.mtx);
      list.producer_state = ERROR;
      pthread_mutex_unlock(&list.mtx);
      break;
    }

    Node *n = Node__init();
    if (n == NULL) {
      free(word);
      pthread_mutex_lock(&list.mtx);
      list.producer_state = ERROR;
      pthread_mutex_unlock(&list.mtx);
      break;
    }

    n->x = x;
    n->word = word;
    List__push(&list, n);

    word = NULL;
  }

  pthread_mutex_lock(&list.mtx);
  list.producer_state = ret != EOF ? ERROR : DONE;
  pthread_cond_broadcast(&list.cond);
  pthread_mutex_unlock(&list.mtx);

  pthread_exit(NULL);
}

void *consumer(void *arg) {
  long id = (long)arg;

  while (1) {
    Node *n = List__pop(&list);
    if (n == NULL) {
      break;
    }

    pthread_mutex_lock(&stdout_mtx);
    printf("Thread %ld:", id); // Print prefix
    for (size_t i = 0; i < n->x; i++) {
      putchar(' ');
      fputs(n->word, stdout);
    }
    putchar('\n');
    pthread_mutex_unlock(&stdout_mtx);

    Node__free(n);
  }

  pthread_exit(NULL);
}
