#include "IO_utils.h"
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>

int map_IO_phys(void **spiled_mem_base, void **parlcd_mem_base) {
  // Map SPI LED line
  *spiled_mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (*spiled_mem_base == NULL) {
    fprintf(stderr, "SPI LED MAPPING FAILED");
    return SPILED_MAP_ERR;
  }

  // Map PAR LCD
  *parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (*spiled_mem_base == NULL) {
    fprintf(stderr, "PAR LCD MAPPING FAILED");
    return PARLCD_MAP_ERR;
  }

  return 0;
}

void set_leds(void *spiled_mem_base, int left_led_color, int right_led_color) {
  *(volatile uint32_t *)((char *)spiled_mem_base + SPILED_REG_LED_RGB1_o) = left_led_color;
  *(volatile uint32_t *)((char *)spiled_mem_base + SPILED_REG_LED_RGB2_o) = right_led_color;
}

void write_led(void *spiled_mem_base, uint32_t value) { *(volatile uint32_t *)((char *)spiled_mem_base + SPILED_REG_LED_LINE_o) = value; }

void write_integer_bitmask(void *spiled_mem_base, int val) { write_led(spiled_mem_base, (uint32_t)((1U << (val % 32)) - 1)); }

void write_lcd_fb(void *parlcd_mem_base, unsigned short *fb) {
  parlcd_write_cmd(parlcd_mem_base, 0x2c);

  // Write the framebuffer to LCD
  const int total_pixels = LCD_W * LCD_H;
  for (int i = 0; i < total_pixels / 2; i++) {
    uint32_t data = ((uint32_t)fb[2 * i + 1] << 16) | fb[2 * i];
    parlcd_write_data2x((unsigned char *)parlcd_mem_base, data);
  }
}

void wipe_fb(unsigned short *fb) {
  for (int x = 0; x < LCD_W; x++) {
    for (int y = 0; y < LCD_H; y++) {
      fb[LCD_W * y + x] = LCD_DEFAULT_COLOR;
    }
  }
}

int button_is_pressed(Button button, void *spiled_mem_base) {
  uint32_t buttons = *(volatile uint32_t *)((char *)spiled_mem_base + SPILED_REG_KNOBS_8BIT_o);
  if (((buttons >> (24 + button)) & 1) == 1) return TRUE;
  return FALSE;
}

int knob_rotation(Knob knob, void *spiled_mem_base) {
  uint32_t knobs = *(volatile uint32_t *)((char *)spiled_mem_base + SPILED_REG_KNOBS_8BIT_o);
  return (knobs >> knob) & 0xFF;
}

int knob_rotation_delta(Knob knob, void *spiled_mem_base, int prev_rotation) {
  int current_rotation = knob_rotation(knob, spiled_mem_base);
  int delta = current_rotation - prev_rotation;
  if (delta > 128) {
    delta -= 256;
  } else if (delta < -128) {
    delta += 256;
  }
  return delta;
}
