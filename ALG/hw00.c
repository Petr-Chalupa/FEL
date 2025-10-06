#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_NUMBER_COUNT 2
#define MAX_NUMBER_COUNT 1000
#define HIGHEST_NUMBER 1000 // the numbers will be in <0;1000>

int main() {
  int numbers[HIGHEST_NUMBER + 1] = {0};
  int count = 0;

  int n;
  while (count < MAX_NUMBER_COUNT && scanf("%d", &n) == 1) {
    if (n >= 0 && n <= HIGHEST_NUMBER) {
      numbers[n]++;
      count++;
    }

    char c = getchar();
    if (c == '\n' || c == EOF) {
      break;
    }
  }

  if (count < MIN_NUMBER_COUNT) {
    fprintf(stderr, "Too few numbers: %d", count);
    return 1;
  }

  bool is_first_duplicate = true;
  for (size_t i = 0; i <= HIGHEST_NUMBER; i++) {
    if (numbers[i] > 1) {
      if (is_first_duplicate) {
        printf("%zu", i);
        is_first_duplicate = false;
      } else {
        printf(" %zu", i);
      }
    }
  }

  printf("\n");

  return 0;
}