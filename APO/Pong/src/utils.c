#include "utils.h"
#include "IO_utils.h"
#include "font_types.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "logo.c"

void pprintf(const char *color, const char *format, ...) {
  va_list args;
  va_start(args, format);
  printf("%s", color);
  vprintf(format, args);
  printf(PRINT_COLOR_END);
  va_end(args);
}

void change_game_state(GameState *old_state, GameState new_state) {
  *old_state = new_state;
  pprintf(PRINT_COLOR_BLUE, "Game state changed to %d\n", new_state);
  // Sleep for debouncing
  struct timespec between_menu_delay = {.tv_sec = 0, .tv_nsec = BETWEEN_MENU_DELAY};
  clock_nanosleep(CLOCK_MONOTONIC, 0, &between_menu_delay, NULL);
}

void render_menu(MenuItem *items, size_t items_count, size_t *sel_idx, void *parlcd_mem_base, unsigned short *fb) {
  wipe_fb(fb);

  int start_y = (LCD_H - (items_count * MENU_ITEM_HEIGHT)) / 2;

  for (size_t i = 0; i < items_count; i++) {
    int y = start_y + (i * MENU_ITEM_HEIGHT);
    unsigned short color = (i == *sel_idx) ? LCD_COLOR_BLUE : LCD_COLOR_WHITE;
    const char *txt = items[i].text;
    render_text(40, y + MENU_ITEM_PADDING, txt, color, fb);
  }

  for (int x = 0; x < logo_png_width; x++) {
    for (int y = 0; y < logo_png_height; y++) {
      fb[(y + MENU_LOGO_Y) * LCD_W + x + MENU_LOGO_X] = logo_png[y * logo_png_height + x];
    }
  }

  write_lcd_fb(parlcd_mem_base, fb);
}

void render_text(int x, int y, const char *text, unsigned short color, unsigned short *fb) {
  font_descriptor_t *font = &font_winFreeSystem14x16;
  int cursor_x = x;
  int height = font->height;

  while (*text) {
    unsigned char c = *text++;
    if (c < font->firstchar || c >= font->firstchar + font->size) c = font->defaultchar;

    int char_index = c - font->firstchar;
    int width = font->width[char_index];
    uint32_t offset = char_index * height;

    for (int row = 0; row < height; row++) {
      font_bits_t row_bits = font->bits[offset + row];

      for (int col = 0; col < width; col++) {
        if (row_bits & (0x8000 >> col)) {
          int px = cursor_x + col;
          int py = y + row;
          if (px >= 0 && px < LCD_W && py >= 0 && py < LCD_H) {
            fb[py * LCD_W + px] = color;
          }
        }
      }
    }
    cursor_x += width + 1; // 1px space
  }
}

void render_text_centered(int y, const char *text, unsigned short color, unsigned short *fb) {
  font_descriptor_t *font = &font_winFreeSystem14x16;

  int text_width = 0;
  for (size_t i = 0; i < strlen(text); i++) {
    int char_index = text[i] - font->firstchar;
    text_width += font->width[char_index];
  }

  int x = (LCD_W - text_width) / 2;
  render_text(x, y, text, color, fb);
}

int get_random_dir(void) { return (rand() % 2 == 0) ? -BALL_SPEED : BALL_SPEED; }

void set_leds_on_new_gamemode(void *spiled_mem_base, GameMode GAMEMODE) {
  switch (GAMEMODE) {
  case PVP:
    set_leds(spiled_mem_base, LED_COLOR_RED, LED_COLOR_BLUE);
    break;
  case DEMO:
    set_leds(spiled_mem_base, LED_COLOR_OFF, LED_COLOR_OFF);
    break;
  case PVC:
    set_leds(spiled_mem_base, LED_COLOR_RED, LED_COLOR_OFF);
    break;
  }
}
