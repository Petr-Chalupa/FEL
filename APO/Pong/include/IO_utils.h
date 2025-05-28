#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdint.h>

#define LCD_W 480 // Width
#define LCD_H 320 // Height

#define LCD_DEFAULT_COLOR 0x0 // Black
#define LCD_COLOR_WHITE 0xFFFF
#define LCD_COLOR_GRAY 0x8410
#define LCD_COLOR_BLUE 0x001F
#define LCD_COLOR_YELLOW 0xFFE0
#define LCD_COLOR_RED 0xF800

#define LED_COLOR_RED 0xFF0000
#define LED_COLOR_BLUE 0x0000FF
#define LED_COLOR_OFF 0x0

typedef enum Button { BUTTON_L = 2, BUTTON_M = 1, BUTTON_R = 0 } Button;

typedef enum Knob { KNOB_L = 16, KNOB_M = 8, KNOB_R = 0 } Knob;

typedef enum IO_ERR { SPILED_MAP_ERR = 100, PARLCD_MAP_ERR = 101 } IO_ERR;

typedef struct {
  unsigned r : 5;
  unsigned g : 6;
  unsigned b : 5;
} __attribute__((packed)) pixel;

/**
 * Maps the physical addresses of SPILED and PARLCD
 * Sets the pointers to the mapped memory base address
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @param parlcd_mem_base: pointer to the PARLCD memory base address
 * @return 0 on success, error code on failure
 */
int map_IO_phys(void **spiled_mem_base, void **parlcd_mem_base);

/**
 * Sets left and right led to provided colours
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @param left_led_color: color to set left led to
 * @param right_led_color: color to set right led to
 */
void set_leds(void *spiled_mem_base, int left_led_color, int right_led_color);

/**
 * Writes 32 bits to the LED line
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @param value: value to write to the LED line
 */
void write_led(void *spiled_mem_base, uint32_t value);

/**
 * Lights up the least significant 'value' bits on the LED line
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @param val: Number of least significant bits to set (0-32)
 */
void write_integer_bitmask(void *spiled_mem_base, int val);

/**
 * Writes framebuffer to the LCD
 * @param parlcd_mem_base: pointer to the PARLCD memory base address
 * @param fb: framebuffer to write
 */
void write_lcd_fb(void *parlcd_mem_base, unsigned short *fb);

/**
 * Resets the framebuffer to the default color
 * @param fb: framebuffer to wipe
 */
void wipe_fb(unsigned short *fb);

/**
 * Check if specified knob button is pressed
 * @param button: button to check
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @return TRUE if pressed, FALSE otherwise
 */
int button_is_pressed(Button button, void *spiled_mem_base);

/**
 * Returns the value of the specified knob
 * @param knob: knob to check
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @return knob value
 */
int knob_rotation(Knob knob, void *spiled_mem_base);

/**
 * Returns the value change of the specified knob
 * @param knob: knob to check
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @param prev_rotation: the previous rotation value
 * @return knob value
 */
int knob_rotation_delta(Knob knob, void *spiled_mem_base, int prev_rotation);

#endif // IO_UTILS_H