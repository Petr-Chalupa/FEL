#include <stdbool.h>
#include <stdio.h>

bool read_numbers(int *a, int *b);
int print_numbers(int a, int b);

int main(int argc, char const *argv[]) {
  int n = 0;
  int m = 0;

  bool readRes = read_numbers(&n, &m);
  if (!readRes) return 100;

  int printRes = print_numbers(n, m);
  return printRes;
}

bool read_numbers(int *a, int *b) {
  bool ret = true;

  int r = scanf("%d %d", a, b);
  if (r < 2) {
    fprintf(stderr, "Chyba nacitani vstupu!\n");
    ret = false;
  } else if (*a < -10000 || *a > 10000 || *b < -10000 || *b > 10000) {
    fprintf(stderr, "Error: Vstup je mimo interval!\n");
    ret = false;
  }

  return ret;
}

int print_numbers(int a, int b) {
  int ret = 0;

  printf("Desitkova soustava: %d %d\n", a, b);
  printf("Sestnactkova soustava: %x %x\n", a, b);

  int sum = a + b;
  printf("Soucet: %d + %d = %d\n", a, b, sum);

  int diff = a - b;
  printf("Rozdil: %d - %d = %d\n", a, b, diff);

  int mul = a * b;
  printf("Soucin: %d * %d = %d\n", a, b, mul);

  if (b == 0) {
    fprintf(stderr, "Error: Nedefinovany vysledek!\n");
    printf("Podil: %d / %d = NaN\n", a, b);
    ret = 101;
  } else {
    int div = a / b;
    printf("Podil: %d / %d = %d\n", a, b, div);
  }

  float avg = (a + b) / 2.f;
  printf("Prumer: %.1f\n", avg);

  return ret;
}