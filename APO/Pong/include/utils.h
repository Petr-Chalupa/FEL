#ifndef UTILS_H
#define UTILS_H

#include "IO_utils.h"
#include "font_types.h"
#include <stddef.h>
#include <stdlib.h>

#define DEBUG_ON 0

#define FRAME_RATE (1000000000L / 30)        // 30 FPS
#define BETWEEN_MENU_DELAY 250 * 1000 * 1000 // 0.25s in ns

#define PRINT_COLOR_RED "\033[1;31m"
#define PRINT_COLOR_GREEN "\033[1;32m"
#define PRINT_COLOR_YELLOW "\033[1;33m"
#define PRINT_COLOR_BLUE "\033[1;34m"
#define PRINT_COLOR_END "\033[0m"

#define MENU_ITEM_PADDING 3
#define MENU_ITEM_HEIGHT font_winFreeSystem14x16.height + 2 * MENU_ITEM_PADDING // Text + top & bottom padding
#define MENU_KNOB_THRESHOLD 1
#define MENU_LOGO_X 245
#define MENU_LOGO_Y 65

#define RACKET_HEIGHT 63
#define RACKET_WIDTH 2
#define BALL_SIZE 5
#define CENTER_LINE_WIDTH 2

#define MAX_HIGHSCORES 5

#define BALL_SPEED 3

typedef enum Bool { FALSE, TRUE } Bool;

typedef enum GameState { MAIN_MENU, HIGHSCORES, GAMEMODE_MENU, GAME_RUNNING, GAME_PAUSED, GAME_OVER, EXIT } GameState;

typedef enum GameMode { PVP, PVC, DEMO } GameMode;

typedef struct {
  short x;
  short y;
  short dx;
  short dy;
} Ball;

typedef struct {
  const char *text;
  union {
    GameState new_state;
    GameMode game_mode;
  } value;
} MenuItem;

/**
 * Prints formatted text with color
 * @param color: ANSI color code
 * @param format: format string
 * @param ...: additional printf arguments
 */
void pprintf(const char *color, const char *format, ...);

/**
 * Changes the game state
 * @param new_state: new game state to set
 */
void change_game_state(GameState *old_state, GameState new_state);

/**
 * Renders menu items on the LCD
 * @param items: array of menu items
 * @param items_count: number of menu items
 * @param sel_idx: index of the selected item
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @param parlcd_mem_base: pointer to the PARLCD memory base address
 * @param fb: framebuffer to write to
 * @param last_knob_rot: pointer to last rotational position of knob
 */
void render_menu(MenuItem *items, size_t items_count, size_t *sel_idx, void *parlcd_mem_base, unsigned short *fb);

/**
 * Renders text on the LCD
 * @param x: x position (top left corner)
 * @param y: y position (top left corner)
 * @param text: text to render
 * @param color: color of the text
 * @param fb: framebuffer to write to
 */
void render_text(int x, int y, const char *text, unsigned short color, unsigned short *fb);

/**
 * Renders text in the center of the LCD
 * @param y: y position (top left corner)
 * @param text: text to render
 * @param color: color of the text
 * @param fb: framebuffer to write to
 */
void render_text_centered(int y, const char *text, unsigned short color, unsigned short *fb);

/**
 * Returns randomized directed speed
 * @return integer - speed - +/- BALL_SPEED
 */
int get_random_dir(void);

/**
 * sets leds on/or based on gamemode - leds indicate playable rackets
 * @param spiled_mem_base: pointer to the SPILED memory base address
 * @param GAMEMODE: set game mode
 */
void set_leds_on_new_gamemode(void *spiled_mem_base, GameMode GAMEMODE);

#endif // UTILS_H