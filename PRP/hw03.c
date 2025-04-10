#include <stdio.h>
#include <stdlib.h>

#define MIN_SIZE 3
#define MAX_SIZE 69

int read_input(int *, int *, int *);
void print_error(int);
void print_roof(int);
void print_house(int, int, int);
void print_fence(int, int);

int main(int argc, char const *argv[]) {
  int width = MIN_SIZE;
  int height = MIN_SIZE;
  int fence = 0; // pos a < height
  int r = read_input(&width, &height, &fence);
  if (r != 0) print_error(r);

  print_roof(width);
  print_house(height, width, fence);

  return 0;
}

int read_input(int *width, int *height, int *fence) {
  int r = scanf("%d %d", width, height);
  if (r < 2) return 100;
  if (*width < MIN_SIZE || *width > MAX_SIZE || *height < MIN_SIZE || *height > MAX_SIZE) return 101;
  if (*width % 2 == 0) return 102;

  if (*width == *height) {
    r = scanf("%d", fence);
    if (r < 1) return 100;
    if (*fence < 1 || *fence >= *height) return 103;
  }

  return 0;
}

void print_error(int err) {
  switch (err) {
  case 100:
    fprintf(stderr, "Error: Chybny vstup!\n");
    exit(100);
    break;
  case 101:
    fprintf(stderr, "Error: Vstup mimo interval!\n");
    exit(101);
    break;
  case 102:
    fprintf(stderr, "Error: Sirka neni liche cislo!\n");
    exit(102);
    break;
  case 103:
    fprintf(stderr, "Error: Neplatna velikost plotu!\n");
    exit(103);
    break;
  }
}

void print_roof(int width) {
  int height = (width - 1) / 2;
  char roof = 'X';
  char roofFill = ' ';

  for (int i = 0; i < height; i++) {
    // print spaces before the roof
    for (int j = 0; j < height - i; j++) {
      putchar(' ');
    }

    // print left side of the roof
    putchar(roof);

    if (i > 0) {
      // print inside of the roof
      for (int j = 0; j < i + (i - 1); j++) {
        putchar(roofFill);
      }
      // print right side of the roof
      putchar(roof);
    }

    printf("\n");
  }
}

void print_house(int height, int width, int fence) {
  int fillSwitch = 1;
  char houseWall = 'X';
  char houseFillNoFence = ' ';
  char houseFillFence[] = {'*', 'o'};

  for (int i = 0; i < height; i++) {
    // print left wall
    putchar(houseWall);

    // print inside of the house
    for (int j = 0; j < width - 2; j++) {
      if (i == 0 || i == height - 1) {
        putchar(houseWall);
      } else if (fence == 0) {
        putchar(houseFillNoFence);
      } else {
        putchar(houseFillFence[fillSwitch]);
        fillSwitch = !fillSwitch;
      }
    }

    // print right wall
    putchar(houseWall);

    // print fence
    if (fence > 0 && i >= height - fence) {
      int fenceEdge = i == height - fence || i == height - 1;
      print_fence(fence, fenceEdge);
    }

    printf("\n");
  }
}

void print_fence(int width, int edge) {
  // fence must end with '|' => for odd width it must start with '|'
  int fenceSwitch = width % 2 == 0;
  // fence edge has '-' between planks
  char fenceFill[] = {'|', edge ? '-' : ' '};

  for (int i = 0; i < width; i++) {
    putchar(fenceFill[fenceSwitch]);
    fenceSwitch = !fenceSwitch;
  }
}
