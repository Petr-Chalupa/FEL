#include <stdio.h>

#define MAX_N 10000
#define MIN_N -10000

int main(int argc, char const *argv[]) {
  int ret = 0;

  int counter = 0;
  int sum = 0;
  int positive = 0;
  int negative = 0;
  int even = 0;
  int odd = 0;
  int min, max;

  while (1) {
    int n;
    int r = scanf("%d", &n);

    // check whether n was read and is in range
    if (r < 1) break;
    if (n < MIN_N || n > MAX_N) {
      printf("\nError: Vstup je mimo interval!\n");
      ret = 100;
      break;
    }

    // increment counters based on the value of n
    counter++;
    sum += n;
    //
    if (n > 0)
      positive++;
    else if (n < 0)
      negative++;
    //
    if (n % 2 == 0)
      even++;
    else
      odd++;
    //
    if (counter == 1 || n > max) max = n;
    if (counter == 1 || n < min) min = n;

    // print number (numbers are separated by commas)
    if (counter == 1)
      printf("%d", n);
    else
      printf(", %d", n);
  }

  // print statistics only if no error occured
  if (!ret) {
    printf("\n");
    printf("Pocet cisel: %d\n", counter);
    printf("Pocet kladnych: %d\n", positive);
    printf("Pocet zapornych: %d\n", negative);
    printf("Procento kladnych: %.2f\n", (positive / (float)counter) * 100);
    printf("Procento zapornych: %.2f\n", (negative / (float)counter) * 100);
    printf("Pocet sudych: %d\n", even);
    printf("Pocet lichych: %d\n", odd);
    printf("Procento sudych: %.2f\n", (even / (float)counter) * 100);
    printf("Procento lichych: %.2f\n", (odd / (float)counter) * 100);
    printf("Prumer: %.2f\n", (sum / (float)counter));
    printf("Maximum: %d\n", max);
    printf("Minimum: %d\n", min);
  }

  return ret;
}
