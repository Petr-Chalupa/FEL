#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ----- Defines -----

#define START_STATE 1
#define END_STATE 2
#define TAPE_PADDING 32
#define INITIAL_QUEUE_CAPACITY 1024

// ----- Typedefs -----

typedef enum { E_MEMALLOC = 100, E_INVALIDINP = 101, E_INPOUTOFRANGE = 102, E_DECODEFAIL = 103 } Error;

typedef unsigned char Symbol;

typedef enum { LEFT = 0, RIGHT = 1 } Direction;

typedef struct {
  int start_state;
  int end_state;
  Direction dir;
  int r_symbol_idx;
  int w_symbol_idx;
} Edge;

typedef struct {
  Edge *data;
  int size;
  int capacity;
} EdgeList;

typedef struct {
  int *cells; // symbol indexes
  int head;   // current head position
  int capacity;
} Tape;

typedef struct {
  int N; // number of states
  int M; // number of symbols
  int K; // length of start word
  Symbol *symbols;
  EdgeList *edges;
  Tape *tape;
  int current_state;
} TM;

typedef struct {
  Tape *tape;
  int state;
  unsigned long tape_hash;
} Configuration;

typedef struct {
  Configuration *data;
  int head;
  int tail;
  int capacity;
  int size;
} Queue;

// ----- Global vars -----

TM *tm = NULL;
Symbol blank_symbol = 0;
int blank_symbol_idx = 0;

// ----- Declarations -----

void Error__throw(Error e);
char *read_line(size_t *size_out);
void read_symbols(TM *tm);
void read_machine_code(TM *tm);
void read_initial_tape(TM *tm);
//
EdgeList *EdgeList__init();
void EdgeList__free(EdgeList *list);
void EdgeList__push(EdgeList *list, Edge *edge);
Edge *EdgeList__get(EdgeList *list, int index);
//
Queue *Queue__init();
void Queue__free(Queue *q);
void Queue__push(Queue *q, Configuration *config);
Configuration *Queue__pop(Queue *q);
bool Queue__is_empty(Queue *q);
//
Tape *Tape__init();
void Tape__free(Tape *t);
int Tape__read(Tape *t);
void Tape__write(Tape *t, int symbol_idx);
void Tape__move(Tape *t, Direction dir);
void Tape__expand(Tape *t, Direction dir);
Tape *Tape__copy(Tape *t);
void Tape__print(Tape *t, Symbol *symbols);
unsigned long Tape__hash(Tape *t);
//
TM *TM__init();
void TM__free(TM *tm);
bool TM__is_deterministic(TM *tm);
void TM__simulate(TM *tm, Tape **final_tape);

// ----- Definitions -----

int main() {
  tm = TM__init();

  // Read params
  if (scanf("%d %d %d\n", &tm->N, &tm->M, &tm->K) != 3) Error__throw(E_INVALIDINP);
  if (tm->N <= 0 || tm->M <= 0 || tm->K <= 0) Error__throw(E_INPOUTOFRANGE);
  //
  read_symbols(tm);
  read_machine_code(tm);
  read_initial_tape(tm);

  bool is_deterministic = TM__is_deterministic(tm);
  printf(is_deterministic ? "D\n" : "N\n");
  //
  Tape *final_tape = NULL;
  TM__simulate(tm, &final_tape);
  Tape__print(final_tape, tm->symbols);

  Tape__free(final_tape);
  TM__free(tm);

  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case E_MEMALLOC: fprintf(stderr, "Error: Memory allocation!\n"); break;
  case E_INVALIDINP: fprintf(stderr, "Error: Invalid argument!\n"); break;
  case E_INPOUTOFRANGE: fprintf(stderr, "Error: Input is out of range!\n"); break;
  case E_DECODEFAIL: fprintf(stderr, "Error: Machine code could not be decoded!\n"); break;
  }

  TM__free(tm);
  exit(e);
}

char *read_line(size_t *size_out) {
  const size_t CHUNK_SIZE = 1 << 20; // 1 MB
  char chunk[CHUNK_SIZE];
  size_t cap = 16 * CHUNK_SIZE;
  size_t size = 0;

  char *buf = malloc(cap * sizeof(char));
  if (buf == NULL) Error__throw(E_MEMALLOC);

  while (true) {
    size_t n = fread(chunk, 1, CHUNK_SIZE, stdin);
    if (n == 0) {
      if (ferror(stdin)) {
        free(buf);
        Error__throw(E_INVALIDINP);
      }
      break; // EOF
    }

    // Scan chunk for newline
    for (size_t i = 0; i < n; i++) {
      if (chunk[i] == '\n') {
        if (size + i + 1 > cap) {
          cap = size + i + 1;
          char *new_buf = realloc(buf, cap);
          if (new_buf == NULL) {
            free(buf);
            Error__throw(E_MEMALLOC);
          }
          buf = new_buf;
        }

        memcpy(buf + size, chunk, i);
        size += i;

        for (size_t j = n - 1; j > i; j--) {
          if (ungetc(chunk[j], stdin) == EOF) Error__throw(E_INVALIDINP);
        }

        *size_out = size;
        return buf;
      }
    }

    // Copy whole chunk otherwise
    if (size + n + 1 > cap) {
      cap = (size + n) * 2;
      char *new_buf = realloc(buf, cap);
      if (new_buf == NULL) {
        free(buf);
        Error__throw(E_MEMALLOC);
      }
      buf = new_buf;
    }
    memcpy(buf + size, chunk, n);
    size += n;
  }

  *size_out = size;
  return buf;
}

void read_symbols(TM *tm) {
  tm->symbols = malloc(tm->M * sizeof(Symbol));
  if (tm->symbols == NULL) Error__throw(E_MEMALLOC);

  for (int i = 0; i < tm->M; i++) {
    if (scanf(" %c", &tm->symbols[i]) != 1) Error__throw(E_INVALIDINP);
  }

  blank_symbol_idx = tm->M;
  if (scanf(" %c\n", &blank_symbol) != 1) Error__throw(E_INVALIDINP);
}

void read_machine_code(TM *tm) {
  size_t size;
  char *code = read_line(&size);
  if (!code) Error__throw(E_DECODEFAIL);

  const char *s = code;
  const char *end = code + size;

  // Skip initial "111"
  if (size < 3) {
    free(code);
    return;
  }
  s += 3;

  while (s < end) {
    // Check for final "111"
    if (end - s >= 3 && s[0] == '1' && s[1] == '1' && s[2] == '1') break;

    // Read word
    int word_blocks[5] = {0};
    for (size_t i = 0; i < 5; i++) {
      while (s < end && *s == '0') {
        word_blocks[i]++;
        s++;
      }
      if (i < 5 - 1) s++; // No '1' separator after last block
    }

    // Skip "11" separator
    if (s < end && *s == '1') s++;
    if (s < end && *s == '1') s++;

    Edge e = {
        .start_state = word_blocks[0],
        .r_symbol_idx = word_blocks[1] - 1,
        .end_state = word_blocks[2],
        .w_symbol_idx = word_blocks[3] - 1,
        .dir = (word_blocks[4] == 1 ? RIGHT : LEFT),
    };
    EdgeList__push(tm->edges, &e);
  }

  free(code);
}

void read_initial_tape(TM *tm) {
  Tape *t = tm->tape;

  t->head = TAPE_PADDING;
  t->capacity = tm->K + TAPE_PADDING * 2;

  t->cells = malloc(t->capacity * sizeof(int));
  if (t->cells == NULL) Error__throw(E_MEMALLOC);

  // Fill tape with blanks
  for (int i = 0; i < t->capacity; i++) {
    t->cells[i] = blank_symbol_idx;
  }

  // Read symbols
  for (int i = 0; i < tm->K; i++) {
    char s;
    if (scanf(" %c", &s) != 1) Error__throw(E_INVALIDINP);

    int symbol_idx = -1;
    for (int k = 0; k < tm->M; k++) {
      if (tm->symbols[k] == s) {
        symbol_idx = k;
        break;
      }
    }
    if (symbol_idx == -1) Error__throw(E_INPOUTOFRANGE);

    t->cells[t->head + i] = symbol_idx;
  }
}

EdgeList *EdgeList__init() {
  EdgeList *list = malloc(sizeof(EdgeList));
  if (list == NULL) Error__throw(E_MEMALLOC);

  list->size = 0;
  list->capacity = 8;

  list->data = malloc(list->capacity * sizeof(Edge));
  if (list->data == NULL) Error__throw(E_MEMALLOC);

  return list;
}

void EdgeList__free(EdgeList *list) {
  if (list == NULL) return;

  free(list->data);
  free(list);
}

void EdgeList__push(EdgeList *list, Edge *edge) {
  if (list->size == list->capacity) {
    list->capacity *= 2;
    Edge *new_data = realloc(list->data, list->capacity * sizeof(Edge));
    if (new_data == NULL) Error__throw(E_MEMALLOC);

    list->data = new_data;
  }

  list->data[list->size++] = *edge;
}

Edge *EdgeList__get(EdgeList *list, int index) {
  if (index < 0 || index >= list->size) return NULL;
  return &list->data[index];
}

Queue *Queue__init() {
  Queue *q = malloc(sizeof(Queue));
  if (q == NULL) Error__throw(E_MEMALLOC);

  q->capacity = INITIAL_QUEUE_CAPACITY;
  q->head = 0;
  q->tail = 0;
  q->size = 0;

  q->data = malloc(q->capacity * sizeof(Configuration));
  if (q->data == NULL) Error__throw(E_MEMALLOC);

  return q;
}

void Queue__free(Queue *q) {
  if (q == NULL) return;

  for (int i = 0; i < q->size; i++) {
    int idx = (q->head + i) % q->capacity;
    Tape__free(q->data[idx].tape);
  }

  free(q->data);
  free(q);
}

void Queue__push(Queue *q, Configuration *config) {
  if (q->size == q->capacity) {
    int new_capacity = q->capacity * 2;
    Configuration *new_data = malloc(new_capacity * sizeof(Configuration));
    if (new_data == NULL) Error__throw(E_MEMALLOC);

    for (int i = 0; i < q->size; i++) {
      new_data[i] = q->data[(q->head + i) % q->capacity];
    }

    free(q->data);
    q->data = new_data;
    q->capacity = new_capacity;
    q->head = 0;
    q->tail = q->size;
  }

  q->data[q->tail] = *config;
  q->tail = (q->tail + 1) % q->capacity;
  q->size++;
}

Configuration *Queue__pop(Queue *q) {
  if (q->size == 0) return NULL;

  Configuration *config = &q->data[q->head];
  q->head = (q->head + 1) % q->capacity;
  q->size--;

  return config;
}

bool Queue__is_empty(Queue *q) { return q->size == 0; }

Tape *Tape__init() {
  Tape *t = malloc(sizeof(Tape));
  if (t == NULL) Error__throw(E_MEMALLOC);

  t->cells = NULL;
  t->head = 0;
  t->capacity = 0;

  return t;
}

void Tape__free(Tape *t) {
  if (t == NULL) return;

  if (t->cells != NULL) free(t->cells);
  free(t);
}

int Tape__read(Tape *t) { return t->cells[t->head]; }

void Tape__write(Tape *t, int symbol_idx) { t->cells[t->head] = symbol_idx; }

void Tape__move(Tape *t, Direction dir) {
  if (dir == LEFT) {
    t->head--;
    if (t->head < 0) Tape__expand(t, dir);
  } else { // RIGHT
    t->head++;
    if (t->head >= t->capacity) Tape__expand(t, dir);
  }
}

void Tape__expand(Tape *t, Direction dir) {
  size_t old_cap = t->capacity;
  size_t new_cap = old_cap + TAPE_PADDING;

  int *new_cells = malloc(new_cap * sizeof(int));
  if (new_cells == NULL) Error__throw(E_MEMALLOC);

  // Fill tape with blanks
  for (size_t i = 0; i < new_cap; i++) {
    new_cells[i] = blank_symbol_idx;
  }

  if (dir == LEFT) {
    int offset = TAPE_PADDING;
    t->head += offset;
    // Shift existing content to the right
    memcpy(&new_cells[offset], t->cells, old_cap * sizeof(int));
  } else { // RIGHT
    // Copy original content unchanged
    memcpy(new_cells, t->cells, old_cap * sizeof(int));
  }

  // Replace cells
  free(t->cells);
  t->cells = new_cells;
  t->capacity = new_cap;
}

Tape *Tape__copy(Tape *t) {
  Tape *new_tape = malloc(sizeof(Tape));
  if (!new_tape) Error__throw(E_MEMALLOC);

  new_tape->head = t->head;
  new_tape->capacity = t->capacity;

  new_tape->cells = malloc(t->capacity * sizeof(int));
  if (new_tape->cells == NULL) Error__throw(E_MEMALLOC);

  memcpy(new_tape->cells, t->cells, t->capacity * sizeof(int));

  return new_tape;
}

void Tape__print(Tape *t, Symbol *symbols) {
  if (t == NULL) return;

  int first = 0;
  int last = t->capacity - 1;

  // Trim leading blanks
  while (first <= last && t->cells[first] == blank_symbol_idx)
    first++;
  while (last >= first && t->cells[last] == blank_symbol_idx)
    last--;

  if (first > last) return;

  // Print symbols
  for (int i = first; i <= last; i++) {
    if (i > first) printf(" ");
    if (t->cells[i] == blank_symbol_idx) {
      printf("%c", blank_symbol);
    } else {
      printf("%c", symbols[t->cells[i]]);
    }
  }
  printf("\n");
}

unsigned long Tape__hash(Tape *t) {
  unsigned long hash = 5381;

  int first = 0;
  int last = t->capacity - 1;

  while (first <= last && t->cells[first] == blank_symbol_idx)
    first++;
  while (last >= first && t->cells[last] == blank_symbol_idx)
    last--;

  for (int i = first; i <= last; i++) {
    hash = ((hash << 5) + hash) + t->cells[i];
  }

  int relative_head = (first <= last) ? (t->head - first) : 0;
  hash = ((hash << 5) + hash) + relative_head;

  return hash;
}

TM *TM__init() {
  TM *tm = malloc(sizeof(TM));
  if (tm == NULL) Error__throw(E_MEMALLOC);

  tm->N = -1;
  tm->M = -1;
  tm->K = -1;
  tm->symbols = NULL;
  tm->edges = EdgeList__init();
  tm->tape = Tape__init();
  tm->current_state = START_STATE;

  return tm;
}

void TM__free(TM *tm) {
  if (tm == NULL) return;

  if (tm->symbols != NULL) free(tm->symbols);
  EdgeList__free(tm->edges);
  Tape__free(tm->tape);

  free(tm);
}

bool TM__is_deterministic(TM *tm) {
  for (int i = 0; i < tm->edges->size; i++) {
    Edge *e1 = EdgeList__get(tm->edges, i);
    for (int j = i + 1; j < tm->edges->size; j++) {
      Edge *e2 = EdgeList__get(tm->edges, j);
      if (e1->start_state == e2->start_state && e1->r_symbol_idx == e2->r_symbol_idx) {
        return false;
      }
    }
  }
  return true;
}

void TM__simulate(TM *tm, Tape **final_tape) {
  Queue *queue = Queue__init();
  Configuration start = {.tape = Tape__copy(tm->tape), .state = START_STATE, .tape_hash = Tape__hash(tm->tape)};
  Queue__push(queue, &start);

  while (!Queue__is_empty(queue)) {
    Configuration *current = Queue__pop(queue);

    if (current->state == END_STATE) {
      *final_tape = Tape__copy(current->tape);
      Tape__free(current->tape);
      break;
    }

    int current_symbol_idx = Tape__read(current->tape);
    for (int i = 0; i < tm->edges->size; i++) {
      Edge *e = EdgeList__get(tm->edges, i);
      if (e->start_state == current->state && e->r_symbol_idx == current_symbol_idx) {
        Tape *new_tape = Tape__copy(current->tape);
        Tape__write(new_tape, e->w_symbol_idx);
        Tape__move(new_tape, e->dir);
        unsigned long new_hash = Tape__hash(new_tape);

        Configuration next = {.tape = new_tape, .state = e->end_state, .tape_hash = new_hash};
        Queue__push(queue, &next);
      }
    }

    Tape__free(current->tape);
  }

  Queue__free(queue);
}