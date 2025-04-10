#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR_INVALID_INPUT 100
#define ERR_INVALID_INPUT_LENGTH 101

const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

typedef struct {
  char *msg;
  size_t length;
  size_t capacity;
} Message;

typedef struct {
  size_t *dist;
  size_t rows;
  size_t columns;
} Distance;

Message Message__init();
bool Message__resize(Message *message);
size_t Message__compare_chartochar(Message *message1, Message *message2);
size_t Message__compare_levensthein(Message *message1, Message *message2);
Distance Distance__init();
bool Distance__resize(Distance *distance, size_t rows, size_t columns);
bool read_input(Message *message);
char rotate_char(char c, size_t offset);
void shift_str(char *str, size_t str_length, size_t offset);
size_t min(size_t a, size_t b, size_t c);
void throw_error(int err_code);

int main(int argc, char *argv[]) {
  bool match_msg_lengths = argc > 1 && strcmp(argv[1], "-prp-optional") == 0 ? false : true;

  Message ciphred_msg = Message__init();
  bool r1 = read_input(&ciphred_msg);
  if (!r1) throw_error(ERR_INVALID_INPUT);

  Message catched_msg = Message__init();
  bool r2 = read_input(&catched_msg);
  if (!r2) throw_error(ERR_INVALID_INPUT);

  if (match_msg_lengths && ciphred_msg.length != catched_msg.length) throw_error(ERR_INVALID_INPUT_LENGTH);

  size_t max_matching_chars = 0;
  size_t min_levensthein_dist = ciphred_msg.length > catched_msg.length ? ciphred_msg.length : catched_msg.length;
  char *closets_match = malloc(ciphred_msg.length * sizeof(char));
  if (closets_match == NULL) throw_error(EXIT_FAILURE);

  size_t shifts = strlen(charset);
  for (size_t i = 0; i < shifts; i++) {
    shift_str(ciphred_msg.msg, ciphred_msg.length, 1);
    if (match_msg_lengths) {
      size_t matching_chars = Message__compare_chartochar(&ciphred_msg, &catched_msg);
      if (matching_chars > max_matching_chars) {
        max_matching_chars = matching_chars;
        strcpy(closets_match, ciphred_msg.msg);
      }
    } else {
      size_t levensthein_dist = Message__compare_levensthein(&ciphred_msg, &catched_msg);
      if (levensthein_dist < min_levensthein_dist) {
        min_levensthein_dist = levensthein_dist;
        strcpy(closets_match, ciphred_msg.msg);
      }
    }
  }

  printf("%s\n", closets_match);
  free(closets_match);
  free(ciphred_msg.msg);
  free(catched_msg.msg);

  return 0;
}

Message Message__init() {
  Message self = {
      .msg = NULL,
      .length = 0,
      .capacity = 0,
  };
  return self;
}

bool Message__resize(Message *message) {
  size_t new_capacity = message->capacity == 0 ? 2 : message->capacity * 2;
  char *tmp_msg = realloc(message->msg, new_capacity * sizeof(char));
  if (tmp_msg == NULL) {
    return false;
  } else {
    message->msg = tmp_msg;
    message->capacity = new_capacity;
    return true;
  }
}

size_t Message__compare_chartochar(Message *message1, Message *message2) {
  size_t matching_chars = 0;
  for (size_t i = 0; i < message1->length; i++) {
    if (message1->msg[i] == message2->msg[i]) matching_chars++;
  }
  return matching_chars;
}

size_t Message__compare_levensthein(Message *message1, Message *message2) {
  Distance distance = Distance__init();
  bool r = Distance__resize(&distance, message1->length, message2->length);
  if (!r) {
    free(distance.dist);
    throw_error(EXIT_FAILURE);
  }

  // initialize all dist elements
  for (size_t i = 0; i < distance.rows; i++) {
    for (size_t j = 0; j < distance.columns; j++) {
      if (i == 0)
        distance.dist[i * distance.columns + j] = j;
      else if (j == 0)
        distance.dist[i * distance.columns + j] = i;
      else
        distance.dist[i * distance.columns + j] = 0;
    }
  }
  // evaluate edit costs
  for (size_t i = 1; i < distance.rows; i++) {
    for (size_t j = 1; j < distance.columns; j++) {
      size_t subst_cost = message1->msg[i - 1] == message2->msg[j - 1] ? 0 : 1;
      distance.dist[i * distance.columns + j] =
          min(distance.dist[(i - 1) * distance.columns + j] + 1,               // deletion
              distance.dist[i * distance.columns + (j - 1)] + 1,               // insertion
              distance.dist[(i - 1) * distance.columns + (j - 1)] + subst_cost // substitution
          );
    }
  }

  size_t result_dist = distance.dist[(distance.rows - 1) * distance.columns + (distance.columns - 1)];
  free(distance.dist);
  return result_dist;
}

Distance Distance__init() {
  Distance self = {
      .dist = NULL,
      .rows = 0,
      .columns = 0,
  };
  return self;
}

bool Distance__resize(Distance *distance, size_t rows, size_t columns) {
  size_t new_capacity = rows * columns;
  size_t *tmp_dist = realloc(distance->dist, new_capacity * sizeof(size_t));
  if (tmp_dist == NULL) {
    return false;
  } else {
    distance->dist = tmp_dist;
    distance->rows = rows;
    distance->columns = columns;
    return true;
  }
}

bool read_input(Message *message) {
  while (true) {
    int c_read = fgetc(stdin);
    if (c_read == EOF) {
      if (feof(stdin)) {
        break;
      } else {
        free(message->msg);
        return false;
      }
    }

    char c = (char)c_read;
    if (c == '\n') break;

    if (strchr(charset, c) == NULL) {
      // character is not defined in charset
      free(message->msg);
      return false;
    }

    // + 1 for string terminating \0 character
    if (message->length + 1 >= message->capacity) {
      bool r = Message__resize(message);
      if (!r) {
        free(message->msg);
        return false;
      }
    }

    message->msg[message->length] = c;
    message->length++;
  }

  message->msg[message->length] = '\0';
  message->length++;
  return true;
}

char rotate_char(char c, size_t offset) {
  char *charset_char = strchr(charset, c);
  size_t char_index = charset_char - charset;
  size_t new_index = (char_index + offset) % strlen(charset);
  return charset[new_index];
}

void shift_str(char *str, size_t str_length, size_t offset) {
  for (size_t i = 0; i < str_length; i++) {
    str[i] = str[i] == '\0' ? '\0' : rotate_char(str[i], offset);
  }
}

size_t min(size_t a, size_t b, size_t c) {
  size_t min = a;
  if (b < min) min = b;
  if (c < min) min = c;
  return min;
}

void throw_error(int err_code) {
  switch (err_code) {
  case ERR_INVALID_INPUT:
    fprintf(stderr, "Error: Chybny vstup!\n");
    break;
  case ERR_INVALID_INPUT_LENGTH:
    fprintf(stderr, "Error: Chybna delka vstupu!\n");
    break;
  default:
    fprintf(stderr, "Error %d has occured\n", err_code);
    break;
  }
  exit(err_code);
}
