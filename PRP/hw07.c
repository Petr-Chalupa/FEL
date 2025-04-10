#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ARG_REGEXP "-E"
#define ARG_COLOR "--color=always"

#define COLOR_START "\033[01;31m\033[K"
#define COLOR_END "\033[m\033[K"

typedef enum {
  ERR_NO_MATCH = 1,
  ERR_INVALID_NUMER_OF_ARGS = 2,
  ERR_INVALID_ARG = 3,
  ERR_MEMORY_ALLOC = 4,
  ERR_FILE_OPEN = 5,
  ERR_FILE_READ = 6,
} Error;

typedef struct {
  char *value;
  size_t length;
  size_t capacity;
  size_t *matches;
  size_t m_length;
  size_t m_capacity;
} Line;

typedef struct {
  Line **lines;
  size_t length;
  size_t capacity;
} Lines;

void Error__throw(Error e);
void set_args(int argc, char const *argv[], bool *arg_regexp, bool *arg_color, char **pattern, char **filename);
bool is_string_equal(char *str1, char *str2);
Line *Line__init();
void Line__free(Line *l);
void Line__resize_value(Line *l);
void Line__resize_matches(Line *l);
int Line__search(Line *l, size_t match_start, char *pattern, bool arg_regexp);
void Line__print(Line *l, bool arg_color);
Lines *Lines__init();
void Lines__free(Lines *s);
void Lines__resize(Lines *l);
void Lines__search(Lines *l, char *pattern, bool arg_regexp);
void Lines__print(Lines *l, bool arg_color);
Lines *read_file(char *filename);

int main(int argc, char const *argv[]) {
  bool arg_regexp = false;
  bool arg_color = false;
  char *pattern = NULL;
  char *filename = NULL;
  set_args(argc, argv, &arg_regexp, &arg_color, &pattern, &filename);

  Lines *lines = read_file(filename);
  Lines__search(lines, pattern, arg_regexp);
  Lines__print(lines, arg_color);
  Lines__free(lines);

  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case ERR_INVALID_NUMER_OF_ARGS:
    fprintf(stderr, "Error: The number of passed arguments is invalid!\n\tUsage: [OPTIONS] PATTERN [FILE]\n");
    break;
  case ERR_INVALID_ARG:
    fprintf(stderr, "Error: The passed argument is invalid!\n\tValid arguments: -E, --color=always\n");
    break;
  case ERR_MEMORY_ALLOC:
    fprintf(stderr, "Error: Memory could not be allocated!\n");
    break;
  case ERR_FILE_OPEN:
    fprintf(stderr, "Error: File could not be opened!\n");
    break;
  case ERR_FILE_READ:
    fprintf(stderr, "Error: File could not be read!\n");
    break;
  default:
    break;
  }
  exit(e);
}

void set_args(int argc, char const *argv[], bool *arg_regexp, bool *arg_color, char **pattern, char **filename) {
  if (argc < 2 || argc > 5) {
    Error__throw(ERR_INVALID_NUMER_OF_ARGS);
  }
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') continue;
    char *arg = (char *)argv[i];
    if (is_string_equal(arg, ARG_REGEXP)) {
      *arg_regexp = true;
    } else if (is_string_equal(arg, ARG_COLOR)) {
      *arg_color = true;
    } else {
      Error__throw(ERR_INVALID_ARG);
    }
  }
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') continue;
    char *arg = (char *)argv[i];
    if (i == argc - 2 || (i == argc - 1 && *pattern == NULL)) {
      *pattern = arg;
    } else if (i == argc - 1) {
      *filename = arg;
    } else {
      Error__throw(ERR_INVALID_ARG);
    }
  }
}

bool is_string_equal(char *str1, char *str2) {
  while (*str1 != '\0' || *str2 != '\0') {
    if (*str1 == *str2) {
      str1++;
      str2++;
    } else {
      return false;
    }
  }
  return true;
}

Line *Line__init() {
  Line *l = malloc(sizeof(Line));
  if (l == NULL) {
    Error__throw(ERR_MEMORY_ALLOC);
  } else {
    l->value = malloc(sizeof(char));
    if (l->value == NULL) {
      free(l);
      Error__throw(ERR_MEMORY_ALLOC);
    }
    l->length = 0;
    l->capacity = 1;
    //
    l->matches = malloc(2 * sizeof(size_t));
    if (l->matches == NULL) {
      free(l);
      Error__throw(ERR_MEMORY_ALLOC);
    }
    l->m_length = 0;
    l->m_capacity = 2;
  }
  return l;
}

void Line__free(Line *l) {
  free(l->value);
  free(l->matches);
  free(l);
}

void Line__resize_value(Line *l) {
  l->value = realloc(l->value, l->capacity * 2 * sizeof(char));
  if (l->value == NULL) {
    Line__free(l);
    Error__throw(ERR_MEMORY_ALLOC);
  } else {
    l->capacity *= 2;
  }
}

void Line__resize_matches(Line *l) {
  l->matches = realloc(l->matches, l->m_capacity * 2 * sizeof(size_t));
  if (l->matches == NULL) {
    Line__free(l);
    Error__throw(ERR_MEMORY_ALLOC);
  } else {
    l->m_capacity *= 2;
  }
}

void Line__add_match(Line *l, size_t start, size_t end) {
  // +1 because 2 items will be added
  if (l->m_length + 1 >= l->m_capacity) Line__resize_matches(l);
  l->matches[l->m_length++] = start;
  l->matches[l->m_length++] = end;
}

int Line__search(Line *l, size_t match_start, char *pattern, bool arg_regexp) {
  size_t pattern_index = 0;
  int match_end = match_start - 1;
  bool plus_prev_matched = false;

  for (size_t i = match_start; i < l->length; i++) {
    char ch = l->value[i];
    if (arg_regexp && pattern[pattern_index + 1] == '?') {
      match_end++;
      pattern_index += 2;
      if (ch == pattern[pattern_index]) i--;
    } else if (arg_regexp && pattern[pattern_index + 1] == '*') {
      if (ch == pattern[pattern_index]) {
        match_end++;
      } else {
        pattern_index += 2;
        i--;
      }
    } else if (arg_regexp && pattern[pattern_index + 1] == '+') {
      if (ch == pattern[pattern_index]) {
        match_end++;
        plus_prev_matched = true;
      } else if (plus_prev_matched) {
        plus_prev_matched = false;
        pattern_index += 2;
        i--;
      } else {
        match_end = -1;
        break;
      }
    } else {
      if (ch == pattern[pattern_index]) {
        match_end++;
        pattern_index++;
      } else {
        match_end = -1;
        break;
      }
    }
    if (pattern[pattern_index] == '\0') {
      break;
    }
  }

  return match_end;
}

void Line__print(Line *l, bool arg_color) {
  size_t next_match = 0;
  for (size_t i = 0; i < l->length - 1; i++) {
    if (arg_color && next_match < l->m_length && i == l->matches[next_match]) printf(COLOR_START);
    putchar(l->value[i]);
    if (arg_color && next_match + 1 < l->m_length && i == l->matches[next_match + 1]) {
      printf(COLOR_END);
      next_match += 2; // skipping end index
    }
  }
  putchar('\n');
}

Lines *Lines__init() {
  Lines *l = malloc(sizeof(Lines));
  if (l == NULL) {
    Error__throw(ERR_MEMORY_ALLOC);
  } else {
    l->lines = malloc(sizeof(Line *));
    if (l->lines == NULL) {
      free(l);
      Error__throw(ERR_MEMORY_ALLOC);
    }
    l->length = 0;
    l->capacity = 1;
  }
  return l;
}

void Lines__free(Lines *l) {
  for (size_t i = 0; i < l->length; i++) {
    Line__free(l->lines[i]);
  }
  free(l->lines);
  free(l);
}

void Lines__resize(Lines *l) {
  l->lines = realloc(l->lines, l->capacity * 2 * sizeof(Line *));
  if (l->lines == NULL) {
    Lines__free(l);
    Error__throw(ERR_MEMORY_ALLOC);
  } else {
    l->capacity *= 2;
  }
}

void Lines__search(Lines *l, char *pattern, bool arg_regexp) {
  for (size_t i = 0; i < l->length; i++) {
    size_t match_start = 0;
    while (match_start < l->lines[i]->length) {
      int match_end = Line__search(l->lines[i], match_start, pattern, arg_regexp);
      if (match_end == -1) {
        match_start++;
      } else {
        Line__add_match(l->lines[i], match_start, match_end);
        match_start = match_end + 1;
      }
    }
  }
}

void Lines__print(Lines *l, bool arg_color) {
  size_t matched_lines = 0;

  for (size_t i = 0; i < l->length; i++) {
    if (l->lines[i]->m_length > 0) {
      Line__print(l->lines[i], arg_color);
      matched_lines++;
    }
  }

  if (matched_lines == 0) {
    Lines__free(l);
    Error__throw(ERR_NO_MATCH);
  }
}

Lines *read_file(char *filename) {
  FILE *file = filename == NULL ? stdin : fopen(filename, "r");
  if (file == NULL) {
    Error__throw(ERR_FILE_OPEN);
  }

  Lines *lines = Lines__init();
  while (true) {
    if (feof(file)) break;

    Line *line = Line__init();
    while (true) {
      char c_read = fgetc(file);
      if (c_read == EOF) {
        if (feof(file)) {
          break; // expected EOF
        } else {
          Lines__free(lines);
          Error__throw(ERR_FILE_READ);
        }
      } else if (c_read == '\n') {
        line->value[line->length++] = '\0';
        break;
      }

      // +1 for \0
      if (line->length + 1 >= line->capacity) Line__resize_value(line);
      line->value[line->length++] = c_read;
    }

    if (lines->length >= lines->capacity) Lines__resize(lines);
    lines->lines[lines->length++] = line;
  }

  if (filename != NULL) fclose(file);
  return lines;
}
