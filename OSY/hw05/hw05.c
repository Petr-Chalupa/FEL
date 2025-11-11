#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _STEP_COUNT 6

// --- Typedefs ---

typedef enum { NUZKY, VRTACKA, OHYBACKA, SVARECKA, LAKOVNA, SROUBOVAK, FREZA, _PLACE_COUNT } Place;

typedef enum { A, B, C, _PRODUCT_COUNT } Product;

typedef struct {
  Product product;
  size_t step;
} Task;

typedef struct {
  pthread_t thread;

  char *name;
  Place place;

  bool is_waiting_for_place; // True if there is no free place available
  bool request_close;        // True if worker's place should be closed
  bool request_leave;        // True if worker is ordered to leave
  bool request_end;          // True if the shift has ended
} Worker;

typedef struct {
  Worker **data;
  size_t size;
  size_t capacity;
} WorkerList;

// --- Constants ---

const char *place_str[_PLACE_COUNT] = {
    [NUZKY] = "nuzky",     [VRTACKA] = "vrtacka",     [OHYBACKA] = "ohybacka", [SVARECKA] = "svarecka",
    [LAKOVNA] = "lakovna", [SROUBOVAK] = "sroubovak", [FREZA] = "freza",
};

const int place_time[_PLACE_COUNT] = {
    [NUZKY] = 100, [VRTACKA] = 200, [OHYBACKA] = 150, [SVARECKA] = 300, [LAKOVNA] = 400, [SROUBOVAK] = 250, [FREZA] = 500,
};

const char *product_str[_PRODUCT_COUNT] = {
    [A] = "A",
    [B] = "B",
    [C] = "C",
};

const int product_steps[_PRODUCT_COUNT][_STEP_COUNT] = {[A] = {NUZKY, VRTACKA, OHYBACKA, SVARECKA, VRTACKA, LAKOVNA},
                                                        [B] = {VRTACKA, NUZKY, FREZA, VRTACKA, LAKOVNA, SROUBOVAK},
                                                        [C] = {FREZA, VRTACKA, SROUBOVAK, VRTACKA, FREZA, LAKOVNA}};

// --- Global vars ---

int free_places[_PLACE_COUNT] = {0};
int taken_places[_PLACE_COUNT] = {0};
int wanted_places[_PLACE_COUNT] = {0};

int products_in_queue[_PRODUCT_COUNT][_STEP_COUNT] = {[A] = {0}, [B] = {0}, [C] = {0}};
int products_in_progress[_PRODUCT_COUNT][_STEP_COUNT] = {[A] = {0}, [B] = {0}, [C] = {0}};

WorkerList *workers = NULL;

bool global_shutdown = false; // For fatal errors

pthread_mutex_t shared_state_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t shared_state_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t io_mtx = PTHREAD_MUTEX_INITIALIZER;

// --- Methods ---

void fprintf_safe(FILE *stream, const char *fmt, ...);
void Error__throw();
int find_string_in_array(const char **array, int length, char *what);
//
Worker *Worker__init(const char *name, Place place);
void Worker__free(Worker *w);
void *Worker__run(void *arg);
bool Worker__has_work(Worker *w);
bool Worker__has_future_work(Worker *w);
void Worker__work(Worker *w);
//
WorkerList *WorkerList__init();
bool WorkerList__resize(WorkerList *wl);
void WorkerList__free(WorkerList *wl);
void WorkerList__add(WorkerList *wList, Worker *w);
void WorkerList_remove(WorkerList *wList, Worker *w);
void WorkerList__end_all(WorkerList *wl);
void WorkerList__join_all(WorkerList *wl);

int main(int argc, char **argv) {
  workers = WorkerList__init();
  if (!workers) {
    Error__throw();
  }

  char *line = NULL;
  size_t sz = 0;

  while (true) {
    char *cmd, *arg1, *arg2, *arg3, *saveptr;

    if (getline(&line, &sz, stdin) == -1) break; /* Error or EOF */

    cmd = strtok_r(line, " \r\n", &saveptr);
    arg1 = strtok_r(NULL, " \r\n", &saveptr);
    arg2 = strtok_r(NULL, " \r\n", &saveptr);
    arg3 = strtok_r(NULL, " \r\n", &saveptr);

    if (!cmd) {
      continue; /* Empty line */
    } else if (strcmp(cmd, "start") == 0 && arg1 && arg2 && !arg3) {
      int place = find_string_in_array(place_str, _PLACE_COUNT, arg2);
      if (place < 0) {
        fprintf_safe(stderr, "Invalid place in start: %s\n", arg2);
        continue;
      }

      Worker *w = Worker__init(arg1, (Place)place);
      if (!w) {
        Error__throw();
      }

      WorkerList__add(workers, w);

      if (pthread_create(&w->thread, NULL, Worker__run, w) != 0) {
        Error__throw();
      }

      pthread_mutex_lock(&shared_state_mtx);
      pthread_cond_broadcast(&shared_state_cond);
      pthread_mutex_unlock(&shared_state_mtx);

    } else if (strcmp(cmd, "make") == 0 && arg1 && !arg2) {
      int product = find_string_in_array(product_str, _PRODUCT_COUNT, arg1);
      if (product < 0) {
        fprintf_safe(stderr, "Invalid product in make: %s\n", arg1);
        continue;
      }

      pthread_mutex_lock(&shared_state_mtx);
      products_in_queue[product][0]++;
      pthread_cond_broadcast(&shared_state_cond);
      pthread_mutex_unlock(&shared_state_mtx);

    } else if (strcmp(cmd, "end") == 0 && arg1 && !arg2) {
      pthread_mutex_lock(&shared_state_mtx);
      for (size_t i = 0; i < workers->size; ++i) {
        Worker *w = workers->data[i];
        if (strcmp(w->name, arg1) == 0) {
          w->request_leave = true;
          pthread_cond_broadcast(&shared_state_cond);
          break;
        }
      }
      pthread_mutex_unlock(&shared_state_mtx);

    } else if (strcmp(cmd, "add") == 0 && arg1 && !arg2) {
      int place = find_string_in_array(place_str, _PLACE_COUNT, arg1);
      if (place < 0) {
        fprintf_safe(stderr, "Invalid place in add: %s\n", arg1);
        continue;
      }

      pthread_mutex_lock(&shared_state_mtx);
      free_places[place]++;
      pthread_cond_broadcast(&shared_state_cond);
      pthread_mutex_unlock(&shared_state_mtx);

    } else if (strcmp(cmd, "remove") == 0 && arg1 && !arg2) {
      int place = find_string_in_array(place_str, _PLACE_COUNT, arg1);
      if (place < 0) {
        fprintf_safe(stderr, "Invalid place in remove: %s\n", arg1);
        continue;
      }

      pthread_mutex_lock(&shared_state_mtx);
      if (free_places[place] > 0) {
        free_places[place]--;
      } else {
        for (size_t i = 0; i < workers->size; i++) {
          Worker *w = workers->data[i];
          if (w->place == place && !w->is_waiting_for_place) {
            w->request_close = true;
            break;
          }
        }
      }
      pthread_cond_broadcast(&shared_state_cond);
      pthread_mutex_unlock(&shared_state_mtx);

    } else {
      fprintf_safe(stderr, "Invalid command: %s\n", line);
    }
  }

  free(line);

  WorkerList__end_all(workers);
  WorkerList__join_all(workers);
  WorkerList__free(workers);

  return 0;
}

void fprintf_safe(FILE *stream, const char *fmt, ...) {
  va_list args;

  pthread_mutex_lock(&io_mtx);
  va_start(args, fmt);
  vfprintf(stream, fmt, args);
  fflush(stream);
  va_end(args);
  pthread_mutex_unlock(&io_mtx);
}

void Error__throw() {
  fprintf_safe(stderr, "Fatal error has occured!\n");

  pthread_mutex_lock(&shared_state_mtx);
  global_shutdown = true;
  pthread_cond_broadcast(&shared_state_cond);
  pthread_mutex_unlock(&shared_state_mtx);

  if (workers) {
    WorkerList__join_all(workers);
    WorkerList__free(workers);
  }

  exit(1);
}

int find_string_in_array(const char **array, int length, char *what) {
  for (int i = 0; i < length; i++)
    if (strcmp(array[i], what) == 0) return i;
  return -1;
}

Worker *Worker__init(const char *name, Place place) {
  Worker *w = malloc(sizeof(Worker));
  if (!w) return NULL;

  w->name = strdup(name);
  if (!w->name) {
    free(w);
    return NULL;
  }

  w->place = place;
  w->is_waiting_for_place = true;
  w->request_close = false;
  w->request_leave = false;
  w->request_end = false;

  pthread_mutex_lock(&shared_state_mtx);
  wanted_places[place]++;
  pthread_mutex_unlock(&shared_state_mtx);

  return w;
}

void Worker__free(Worker *w) {
  if (w) {
    free(w->name);
    free(w);
  }
}

void *Worker__run(void *arg) {
  Worker *w = arg;

  pthread_mutex_lock(&shared_state_mtx);

  while (true) {
    if (global_shutdown || w->request_leave || (w->request_end && !Worker__has_future_work(w))) {
      if (!w->is_waiting_for_place) {
        free_places[w->place]++;
        taken_places[w->place]--;
        wanted_places[w->place]++;
      }
      break;
    }

    if (w->request_close && !w->is_waiting_for_place) {
      taken_places[w->place]--;
      wanted_places[w->place]++;
      w->is_waiting_for_place = true;
      w->request_close = false;
      pthread_cond_broadcast(&shared_state_cond);
      continue;
    }

    if (w->is_waiting_for_place) {
      if (free_places[w->place] > 0) {
        free_places[w->place]--;
        taken_places[w->place]++;
        wanted_places[w->place]--;
        w->is_waiting_for_place = false;
        pthread_cond_broadcast(&shared_state_cond);
        continue;
      }
    }

    if (!w->is_waiting_for_place && Worker__has_work(w)) {
      Worker__work(w);
      pthread_cond_broadcast(&shared_state_cond);
      continue;
    }

    pthread_cond_wait(&shared_state_cond, &shared_state_mtx);
  }

  pthread_cond_broadcast(&shared_state_cond);
  pthread_mutex_unlock(&shared_state_mtx);

  fprintf_safe(stdout, "%s goes home\n", w->name);

  return NULL;
}

bool Worker__has_work(Worker *w) {
  for (int j = _STEP_COUNT - 1; j >= 0; j--) {
    for (int i = 0; i < _PRODUCT_COUNT; i++) {
      if (product_steps[i][j] == w->place && products_in_queue[i][j] > 0) {
        return true;
      }
    }
  }

  return false;
}

bool Worker__has_future_work(Worker *w) {
  for (int i = 0; i < _PRODUCT_COUNT; i++) {
    int last_prod_step = -1;

    for (int j = 0; j < _STEP_COUNT; j++) {
      if (products_in_queue[i][j] > 0) last_prod_step = j;
      if (products_in_progress[i][j] > 0) last_prod_step = j + 1;

      if (product_steps[i][j] != w->place) continue;
      if (last_prod_step == -1) continue;

      bool path_possible = true;
      for (int k = j; k >= last_prod_step; k--) {
        int step_place = product_steps[i][k];
        if (!(taken_places[step_place] > 0 || (free_places[step_place] > 0 && wanted_places[step_place] > 0))) {
          path_possible = false;
          break;
        }
      }
      if (path_possible) return true;
    }
  }

  return false;
}

void Worker__work(Worker *w) {
  Product product = A;
  int step = -1;

  for (int i = 0; i < _PRODUCT_COUNT; i++) {
    for (int j = _STEP_COUNT - 1; j >= 0; j--) {
      if (product_steps[i][j] == w->place && products_in_queue[i][j] > 0) {
        if (step == -1 || j > step || (j == step && i < product)) {
          product = i;
          step = j;
        }
      }
    }
  }

  if (step == -1) return;

  products_in_queue[product][step]--;
  products_in_progress[product][step]++;
  fprintf_safe(stdout, "%s %s %d %s\n", w->name, place_str[w->place], step + 1, product_str[product]);

  pthread_mutex_unlock(&shared_state_mtx);
  usleep(place_time[w->place] * 1000);
  pthread_mutex_lock(&shared_state_mtx);

  products_in_progress[product][step]--;
  if (step + 1 < _STEP_COUNT) {
    products_in_queue[product][step + 1]++;
  } else {
    fprintf_safe(stdout, "done %s\n", product_str[product]);
  }
}

WorkerList *WorkerList__init() {
  WorkerList *wl = malloc(sizeof(WorkerList));
  if (wl == NULL) return NULL;

  wl->data = NULL;
  wl->capacity = 0;
  wl->size = 0;

  return wl;
}

bool WorkerList__resize(WorkerList *wl) {
  size_t new_capacity = wl->capacity == 0 ? 4 : wl->capacity * 2;
  Worker **new_data = realloc(wl->data, new_capacity * sizeof(Worker *));
  if (new_data == NULL) return false;

  wl->data = new_data;
  wl->capacity = new_capacity;

  return true;
}

void WorkerList__free(WorkerList *wl) {
  if (!wl) return;

  for (size_t i = 0; i < wl->size; i++) {
    Worker__free(wl->data[i]);
  }

  free(wl->data);
  free(wl);
}

void WorkerList__add(WorkerList *wList, Worker *w) {
  pthread_mutex_lock(&shared_state_mtx);

  if (wList->size + 1 > wList->capacity) {
    if (!WorkerList__resize(wList)) {
      pthread_mutex_unlock(&shared_state_mtx);
      Worker__free(w);
      Error__throw();
    }
  }

  wList->data[wList->size++] = w;

  pthread_mutex_unlock(&shared_state_mtx);
}

void WorkerList_remove(WorkerList *wList, Worker *w) {
  pthread_mutex_lock(&shared_state_mtx);

  for (size_t i = 0; i < wList->size; i++) {
    if (wList->data[i] == w) {
      for (size_t j = i; j < wList->size - 1; j++) {
        wList->data[j] = wList->data[j + 1];
      }
      wList->size--;
      break;
    }
  }

  pthread_mutex_unlock(&shared_state_mtx);
}

void WorkerList__end_all(WorkerList *wl) {
  pthread_mutex_lock(&shared_state_mtx);

  for (size_t i = 0; i < wl->size; i++) {
    wl->data[i]->request_end = true;
  }

  pthread_cond_broadcast(&shared_state_cond);
  pthread_mutex_unlock(&shared_state_mtx);
}

void WorkerList__join_all(WorkerList *wl) {
  for (size_t i = 0; i < wl->size; i++) {
    pthread_join(wl->data[i]->thread, NULL);
  }
}