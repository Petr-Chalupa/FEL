#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR_INVALID_INPUT 100
#define PRIMES_GEN_LIMIT 1000000
#define NUMBER_MAX_LENGTH 100

void generatePrimes(int *, int *);
void shrinkPrimesArr(int *, int *);
void scanInput(char *);
void generatePrimeFactorization(int *, int, char *);
int divideLargeNumber(char *, int);
void printErrorAndExit(int);

int main() {
  static int sieve[PRIMES_GEN_LIMIT];
  int primesCount = 0;
  generatePrimes(sieve, &primesCount);
  int primes[primesCount];
  shrinkPrimesArr(sieve, primes);

  while (1) {
    char number[NUMBER_MAX_LENGTH + 1]; // + 1 for \0
    scanInput(number);

    if (strcmp(number, "0") == 0)
      break; // EOI
    else {
      generatePrimeFactorization(primes, primesCount, number);
    }
  }

  return 0;
}

void generatePrimes(int *sieve, int *primesCount) {
  // perform the Sieve of Eratosthenes,
  // 2 is the first prime,
  // primes and indexes are not synced

  for (size_t i = 1; i < PRIMES_GEN_LIMIT; i++) {
    sieve[i] = 1;
  }

  for (size_t i = 2; i * i <= PRIMES_GEN_LIMIT; i++) {
    if (sieve[i - 1] == 0) continue;
    for (size_t j = i * i; j <= PRIMES_GEN_LIMIT; j += i) {
      sieve[j - 1] = 0;
    }
  }

  for (size_t i = 0; i < PRIMES_GEN_LIMIT; i++) {
    if (sieve[i] == 1) (*primesCount)++;
  }
}

void shrinkPrimesArr(int *sieve, int *primes) {
  int primeIndex = 0;
  for (size_t i = 0; i < PRIMES_GEN_LIMIT; i++) {
    if (sieve[i] == 1) primes[primeIndex++] = i + 1;
  }
}

void scanInput(char *number) {
  char *r = fgets(number, NUMBER_MAX_LENGTH + 1, stdin);
  if (r == NULL || number[0] == '-') printErrorAndExit(ERR_INVALID_INPUT); // nothing read or negative
  //
  int i = 0; // number length
  while (number[i] != '\n' && number[i] != '\0') {
    if (number[i] - '0' > 9) printErrorAndExit(ERR_INVALID_INPUT); // non-numeric char
    i++;
  }
  if (number[i] == '\n') number[i] = '\0'; // set string end
}

void generatePrimeFactorization(int *primes, int primesCount, char *number) {
  printf("Prvociselny rozklad cisla %s je:\n", number);

  if (strcmp(number, "1") == 0) {
    printf("1\n"); // 1 is a special case
    return;
  }

  int factorsCount = 0;
  int factorIndex = 0;
  int factor = primes[factorIndex];
  int factorPower = 0;
  while (1) {
    if (divideLargeNumber(number, factor) == 0) {
      factorPower++;
    } else {
      if (factorPower > 0) {
        factorsCount++;
        if (factorsCount > 1) {
          printf(" x ");
        }
        if (factorPower == 1) {
          printf("%d", factor);
        } else {
          printf("%d^%d", factor, factorPower);
        }
      }
      if (strcmp(number, "1") == 0 || ++factorIndex >= primesCount) break;
      factor = primes[factorIndex];
      factorPower = 0;
    }
  }
  printf("\n");
}

int divideLargeNumber(char *number, int divider) {
  char res[NUMBER_MAX_LENGTH + 1] = {[0] = '\0'};
  int numLength = strlen(number);
  int numDigitIndex = 0;
  int remainder = number[0] - '0';

  while (numDigitIndex < numLength - 1 && remainder < divider) {
    remainder = remainder * 10 + (number[++numDigitIndex] - '0'); // add next digit
  }
  while (numDigitIndex < numLength) {
    // divide number
    char division[2] = {(remainder / divider) + '0', '\0'};
    strcat(res, division);
    // add next digit
    remainder = remainder % divider;
    if (++numDigitIndex < numLength) {
      remainder = remainder * 10 + (number[numDigitIndex] - '0');
    }
  }

  if (remainder == 0) {
    strcpy(number, res);
  }
  return remainder;
}

void printErrorAndExit(int code) {
  switch (code) {
  case ERR_INVALID_INPUT:
    fprintf(stderr, "Error: Chybny vstup!\n");
    break;
  }
  exit(code);
}
