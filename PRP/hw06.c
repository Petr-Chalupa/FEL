#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_DEFINED_MATRIXES 26

typedef enum {
  ERR_INVALID_INPUT = 100,
  ERR_MEMORY_ALLOCATION = 101,
  ERR_STACK_OVERFLOW = 102,
  ERR_STACK_UNDERFLOW = 103
} Error;

typedef enum { ADD = -1, SUB = -2, MUL = -3 } Operator;

typedef struct {
  int *vals;
  int head;
  size_t capacity;
} Stack;

typedef struct {
  int *vals;
  size_t vals_length;
  size_t capacity;
  size_t rows;
} Matrix;

typedef struct {
  Matrix **matrixes; // first MAX_DEFINED_MATRIXES for defined, next for intermediate results
  size_t last_intermediate_matrix;
  size_t capacity;
  Stack *postfix; // Matrix indexes & Operators
} Expression;

void Error__throw(Error e);
bool char_is_operator(char c);
size_t char_to_index(char c);
int Operator__compare(Operator o1, Operator o2);
Stack *Stack__init();
void Stack__push(Stack *s, int item);
int Stack__head(Stack *s);
int Stack__pop(Stack *s);
bool Stack__has(Stack *s, int item, size_t search_start_index);
void Stack__free(Stack *s);
Matrix *Matrix__read();
void Matrix__print(Matrix *m);
Matrix *Matrix__init();
void Matrix__resize(Matrix *m);
void Matrix__free(Matrix *m);
Matrix *Matrix__add(Matrix *m1, Matrix *m2);
Matrix *Matrix__sub(Matrix *m1, Matrix *m2);
Matrix *Matrix__mul(Matrix *m1, Matrix *m2);
Expression *Expression__init();
void Expression__free(Expression *e);
void Expression__read(Expression *e);
Matrix *Expression__eval(Expression *e);

int main() {
  Expression *e = Expression__init();
  Expression__read(e);

  Matrix *res = Expression__eval(e);
  Matrix__print(res);

  Expression__free(e);

  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case ERR_INVALID_INPUT:
    fprintf(stderr, "Error: Chybny vstup!\n");
    break;
  default:
    fprintf(stderr, "Error: Error %d has occured!\n", e);
    break;
  }
  exit(e);
}

bool char_is_operator(char c) {
  switch (c) {
  case '+':
  case '-':
  case '*':
    return true;
  default:
    return false;
  }
}

size_t char_to_index(char c) {
  size_t index = c - 'A';
  if (index > 25) {
    Error__throw(ERR_INVALID_INPUT);
  }
  return index;
}

int Operator__compare(Operator o1, Operator o2) {
  if (o1 == o2) {
    return 0; // same priority
  } else if (o1 == MUL) {
    return 1; // higher priority
  } else {
    return -1; // lower priority
  }
}

Stack *Stack__init() {
  Stack *s = malloc(sizeof(Stack));
  if (s == NULL) {
    Error__throw(ERR_MEMORY_ALLOCATION);
  } else {
    s->vals = malloc(sizeof(int));
    if (s->vals == NULL) {
      free(s);
      Error__throw(ERR_MEMORY_ALLOCATION);
    }
    s->head = -1;
    s->capacity = 1;
  }
  return s;
}

void Stack__push(Stack *s, int item) {
  if (s->head + 1 >= s->capacity) {
    s->vals = realloc(s->vals, s->capacity * 2 * sizeof(int));
    if (s->vals == NULL) {
      free(s);
      Error__throw(ERR_MEMORY_ALLOCATION);
    }
    s->capacity *= 2;
  }
  s->head++;
  s->vals[s->head] = item;
}

int Stack__head(Stack *s) {
  if (s->head == -1) {
    Error__throw(ERR_STACK_UNDERFLOW);
  }
  return s->vals[s->head];
}

int Stack__pop(Stack *s) {
  if (s->head == -1) {
    Error__throw(ERR_STACK_UNDERFLOW);
  }
  return s->vals[s->head--];
}

bool Stack__has(Stack *s, int item, size_t search_start_index) {
  for (size_t i = search_start_index; i <= s->head; i++) {
    if (s->vals[i] == item) return true;
  }
  return false;
}

void Stack__free(Stack *s) {
  free(s->vals);
  free(s);
}

Matrix *Matrix__read() {
  Matrix *m = Matrix__init();

  while (true) {
    int c_read = fgetc(stdin);
    if (c_read == EOF) {
      Matrix__free(m);
      Error__throw(ERR_INVALID_INPUT);
    } else if (c_read == '\n') {
      break;
    } else if (c_read == ']') {
      m->rows++;
      continue; // end of matrix definition
    } else if (c_read == ';') {
      m->rows++;
    } else if (c_read == '=' || c_read == '[') {
      continue; // ignore
    } else {
      ungetc(c_read, stdin); // move pointer back
    }

    int value;
    if (scanf("%d", &value) != 1) {
      Matrix__free(m);
      Error__throw(ERR_INVALID_INPUT);
    }
    if (m->vals_length >= m->capacity) {
      Matrix__resize(m);
    }
    m->vals[m->vals_length++] = value;
  }

  return m;
}

void Matrix__print(Matrix *m) {
  putchar('[');
  for (size_t i = 0; i < m->vals_length; i++) {
    printf("%d", m->vals[i]);
    if ((i + 1) % (m->vals_length / m->rows) == 0 && i < m->vals_length - 1) putchar(';');
    if (i < m->vals_length - 1) putchar(' ');
  }
  printf("]\n");
}

Matrix *Matrix__init() {
  Matrix *m = malloc(sizeof(Matrix));
  if (m == NULL) {
    Error__throw(ERR_MEMORY_ALLOCATION);
  } else {
    m->vals = malloc(sizeof(int));
    if (m->vals == NULL) {
      free(m);
      Error__throw(ERR_MEMORY_ALLOCATION);
    }
    m->capacity = 1;
    m->vals_length = 0;
    m->rows = 0;
  }
  return m;
}

void Matrix__resize(Matrix *m) {
  m->vals = realloc(m->vals, m->capacity * 2 * sizeof(int));
  if (m->vals == NULL) {
    free(m);
    Error__throw(ERR_MEMORY_ALLOCATION);
  } else {
    m->capacity *= 2;
  }
}

void Matrix__free(Matrix *m) {
  free(m->vals);
  free(m);
}

Matrix *Matrix__add(Matrix *m1, Matrix *m2) {
  if (m1->rows != m2->rows || m1->vals_length != m2->vals_length) {
    printf("%zu %zu\n", m1->rows, m2->rows);
    Error__throw(ERR_INVALID_INPUT);
  }

  Matrix *res = Matrix__init();
  res->rows = m1->rows;

  for (size_t i = 0; i < m1->vals_length; i++) {
    if (i >= res->capacity) Matrix__resize(res);
    res->vals[i] = m1->vals[i] + m2->vals[i];
    res->vals_length++;
  }
  return res;
}

Matrix *Matrix__sub(Matrix *m1, Matrix *m2) {
  if (m1->rows != m2->rows || m1->vals_length != m2->vals_length) {
    Error__throw(ERR_INVALID_INPUT);
  }

  Matrix *res = Matrix__init();
  res->rows = m1->rows;

  for (size_t i = 0; i < m1->vals_length; i++) {
    if (i >= res->capacity) Matrix__resize(res);
    res->vals[i] = m1->vals[i] - m2->vals[i];
    res->vals_length++;
  }
  return res;
}

Matrix *Matrix__mul(Matrix *m1, Matrix *m2) {
  if (m1->vals_length / m1->rows != m2->rows) {
    Error__throw(ERR_INVALID_INPUT);
  }

  Matrix *res = Matrix__init();
  res->rows = m1->rows;

  for (size_t i = 0; i < m1->rows; i++) {
    for (size_t j = 0; j < m2->vals_length / m2->rows; j++) {
      int sum = 0;
      for (size_t k = 0; k < m2->rows; k++) {
        sum += m1->vals[i * (m1->vals_length / m1->rows) + k] * m2->vals[k * (m2->vals_length / m2->rows) + j];
      }
      if (i * (m2->vals_length / m2->rows) + j >= res->capacity) Matrix__resize(res);
      res->vals[i * (m2->vals_length / m2->rows) + j] = sum;
      res->vals_length++;
    }
  }

  return res;
}

Expression *Expression__init() {
  Expression *e = malloc(sizeof(Expression));
  if (e == NULL) {
    Error__throw(ERR_MEMORY_ALLOCATION);
  } else {
    e->matrixes = calloc(26, sizeof(Matrix *));
    if (e->matrixes == NULL) {
      free(e);
      Error__throw(ERR_MEMORY_ALLOCATION);
    }
    e->last_intermediate_matrix = MAX_DEFINED_MATRIXES;
    e->capacity = MAX_DEFINED_MATRIXES;
    e->postfix = Stack__init();
  }
  return e;
}

void Expression__resize(Expression *e) {
  e->matrixes = realloc(e->matrixes, e->capacity * 2 * sizeof(Matrix *));
  if (e->matrixes == NULL) {
    Expression__free(e);
    Error__throw(ERR_MEMORY_ALLOCATION);
  } else {
    e->capacity *= 2;
  }
}

void Expression__free(Expression *e) {
  for (size_t i = 0; i < e->last_intermediate_matrix; i++) {
    if (e->matrixes[i] != NULL) Matrix__free(e->matrixes[i]);
  }
  free(e->matrixes);
  Stack__free(e->postfix);
  free(e);
}

void Expression__read(Expression *e) {
  bool reading_type = 1; // 1 = Matrix; 0 = Expression
  Stack *operator_stack = Stack__init();

  while (true) {
    int c_read = fgetc(stdin);
    if (c_read == EOF) {
      if (feof(stdin)) {
        break; // expected EOF
      } else {
        Expression__free(e);
        Error__throw(ERR_INVALID_INPUT);
      }
    }

    else if (c_read == '\n') {
      if (reading_type == 0) {
        break;
      } else {
        reading_type = !reading_type;
        continue;
      }
    }

    if (reading_type == 1) {
      Matrix *m = Matrix__read();
      size_t index = char_to_index(c_read);
      e->matrixes[index] = m;
    } else {
      if (char_is_operator(c_read)) {
        Operator o;
        switch (c_read) {
        case '+':
          o = ADD;
          break;
        case '-':
          o = SUB;
          break;
        case '*':
          o = MUL;
          break;
        }
        if (operator_stack->head == -1 || Operator__compare(o, Stack__head(operator_stack)) == 1) {
          Stack__push(operator_stack, o);
        } else {
          while (operator_stack->head != -1 && Operator__compare(o, Stack__head(operator_stack)) != 1) {
            Stack__push(e->postfix, Stack__pop(operator_stack));
          }
          Stack__push(operator_stack, o);
        }
      } else {
        Stack__push(e->postfix, char_to_index(c_read));
      }
    }
  }

  while (operator_stack->head != -1) {
    Stack__push(e->postfix, Stack__pop(operator_stack));
  }
  Stack__free(operator_stack);
}

Matrix *Expression__eval(Expression *e) {
  Stack *operand_stack = Stack__init();

  for (size_t i = 0; i < e->postfix->head + 1; i++) {
    int value = e->postfix->vals[i];
    if (value < 0) {
      Matrix *res = NULL;
      size_t m1_index = Stack__pop(operand_stack);
      Matrix *m1 = e->matrixes[m1_index];
      size_t m2_index = Stack__pop(operand_stack);
      Matrix *m2 = e->matrixes[m2_index];
      switch (value) {
      case ADD:
        res = Matrix__add(m2, m1);
        break;
      case SUB:
        res = Matrix__sub(m2, m1);
        break;
      case MUL:
        res = Matrix__mul(m2, m1);
        break;
      }
      if (!Stack__has(operand_stack, m1_index, 0) && !Stack__has(e->postfix, m1_index, i)) {
        Matrix__free(m1);
        e->matrixes[m1_index] = NULL;
      }
      if (!Stack__has(operand_stack, m2_index, 0) && !Stack__has(e->postfix, m2_index, i)) {
        Matrix__free(m2);
        e->matrixes[m2_index] = NULL;
      }
      if (e->last_intermediate_matrix >= e->capacity) Expression__resize(e);
      e->matrixes[e->last_intermediate_matrix] = res;
      Stack__push(operand_stack, e->last_intermediate_matrix++);
    } else {
      Stack__push(operand_stack, value);
    }
  }

  Matrix *result = e->matrixes[Stack__pop(operand_stack)];
  Stack__free(operand_stack);
  return result;
}
