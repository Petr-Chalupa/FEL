#include <stdint.h>
#include <stdio.h>

#define LCD_FB_START ((struct pixel *)0xffe00000)
#define LCD_FB_END ((struct pixel *)0xffe4afff)

struct pixel {
  unsigned r : 5;
  unsigned g : 6;
  unsigned b : 5;
} __attribute__((packed));

int main(int argc, char const *argv[]) {
  struct pixel *fb = (void *)LCD_FB_END;

  for (size_t x = 0; x < 480; x++) {
    for (size_t y = 0; y < 320; y++) {
      (fb + x + y * 480)->r = 0x1f;
    }
  }

  return 0;
}
