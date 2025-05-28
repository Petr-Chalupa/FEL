/*******************************************************************

  pong.c - main file

  Authors: Petr Chalupa, Jan Tomasik

 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "IO_utils.h"
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "serialize_lock.h"
#include "utils.h"

#define MAIN_MENU_ITEMS_COUNT 3
MenuItem MAIN_MENU_ITEMS[MAIN_MENU_ITEMS_COUNT] = {
    {"Start Game", {.new_state = GAMEMODE_MENU}},
    {"High Scores", {.new_state = HIGHSCORES}},
    {"Exit", {.new_state = EXIT}},
};

#define GAMEMODE_MENU_ITEMS_COUNT 3
MenuItem GAMEMODE_MENU_ITEMS[GAMEMODE_MENU_ITEMS_COUNT] = {
    {"PVP", {.game_mode = PVP}},
    {"PVC", {.game_mode = PVC}},
    {"DEMO", {.game_mode = DEMO}},
};

#define PAUSE_MENU_ITEMS_COUNT 2
MenuItem PAUSE_MENU_ITEMS[PAUSE_MENU_ITEMS_COUNT] = {
    {"Resume", {.new_state = GAME_RUNNING}},
    {"Menu", {.new_state = MAIN_MENU}},
};

// IO vars
void *spiled_mem_base;
void *parlcd_mem_base;
unsigned short *fb;
// Game loop vars
struct timespec last_frame_time;
struct timespec current_time;
// Game state vars
GameState GAMESTATE = MAIN_MENU;
GameMode GAMEMODE = PVP;
size_t menu_sel_idx = 0;
int menu_knob_rotation = 0;
Ball ball = {.x = LCD_W / 2, .y = LCD_H / 2, .dx = 0, .dy = 0};
int left_racket_pos = 0;
int right_racket_pos = 0;
int left_score = 0;
int right_score = 0;
int high_scores[MAX_HIGHSCORES] = {0, 0, 0, 0, 0};
int last_collision = 100;

void game_loop();
void update();
void render();

void render_high_scores();
void render_game();
void render_game_over();
void draw_racket(Knob player, int pos);
void draw_ball();
void draw_score();
void draw_center_line();
void draw_goodbye_screen();

void update_game_state();
void update_menu_sel_idx();
void update_racket_pos();
void update_computer_racket_pos(int *racket_pos, int error);
void update_ball_pos();
int get_player_position(Knob player);
Bool ball_hit_racket(Knob player, int pos);
Bool ball_not_caught();
void update_high_scores(int score);
void reset_ball();
void reset_scores();

int main() {
  // Acquire the lock first
  if (serialize_lock(1) <= 0) {
    pprintf(PRINT_COLOR_YELLOW, "System is occupied, waiting\n");
    serialize_lock(0);
  }

  // Map IO addresses
  int map_r = map_IO_phys(&spiled_mem_base, &parlcd_mem_base);
  if (map_r != 0) return map_r;

  // Initialize the LCD framebuffer
  fb = malloc(LCD_W * LCD_H * sizeof(unsigned short));

  parlcd_hx8357_init(parlcd_mem_base);

  // init starter ball dirs
  ball.dx = get_random_dir();
  ball.dy = get_random_dir();

  // Start the game loop
  game_loop();
  free(fb);

  // Release the lock
  serialize_unlock();
  return 0;
}

/* --------------------------------- GAME LOOP --------------------------------- */

void game_loop() {
  pprintf(PRINT_COLOR_GREEN, "Starting...\n");
  clock_gettime(CLOCK_MONOTONIC, &last_frame_time);

  while (TRUE) {
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    long delta_time = (current_time.tv_sec - last_frame_time.tv_sec) * 1000000000L + (current_time.tv_nsec - last_frame_time.tv_nsec);

    if (delta_time >= FRAME_RATE) {
      update();
      render();

      last_frame_time = current_time;

      if (GAMESTATE == EXIT) {
        draw_goodbye_screen();
        pprintf(PRINT_COLOR_RED, "Exitting...\n");
        break;
      }
    }
  }
}

void update() {
  update_game_state();

  if (GAMESTATE == MAIN_MENU || GAMESTATE == GAMEMODE_MENU || GAMESTATE == GAME_PAUSED) {
    update_menu_sel_idx();
  }

  if (GAMESTATE == GAME_RUNNING) {
    update_racket_pos();
    update_ball_pos();
  }
}

void render() {
  switch (GAMESTATE) {
  case MAIN_MENU:
    render_menu(MAIN_MENU_ITEMS, MAIN_MENU_ITEMS_COUNT, &menu_sel_idx, parlcd_mem_base, fb);
    break;
  case HIGHSCORES:
    render_high_scores();
    break;
  case GAMEMODE_MENU:
    render_menu(GAMEMODE_MENU_ITEMS, GAMEMODE_MENU_ITEMS_COUNT, &menu_sel_idx, parlcd_mem_base, fb);
    break;
  case GAME_RUNNING:
    render_game();
    break;
  case GAME_PAUSED:
    render_menu(PAUSE_MENU_ITEMS, PAUSE_MENU_ITEMS_COUNT, &menu_sel_idx, parlcd_mem_base, fb);
    break;
  case GAME_OVER:
    render_game_over();
    break;
  default:
    break;
  }
}

/* --------------------------------- UPDATING ---------------------------------- */

void update_game_state() {
  if (!button_is_pressed(BUTTON_M, spiled_mem_base)) return;

  switch (GAMESTATE) {
  case MAIN_MENU:
    change_game_state(&GAMESTATE, MAIN_MENU_ITEMS[menu_sel_idx].value.new_state);
    reset_ball();
    reset_scores();
    menu_sel_idx = 0;
    break;
  case GAMEMODE_MENU:
    GAMEMODE = GAMEMODE_MENU_ITEMS[menu_sel_idx].value.game_mode;
    set_leds_on_new_gamemode(spiled_mem_base, GAMEMODE);
    change_game_state(&GAMESTATE, GAME_RUNNING);
    menu_sel_idx = 0;
    break;
  case GAME_PAUSED:
    change_game_state(&GAMESTATE, PAUSE_MENU_ITEMS[menu_sel_idx].value.new_state);
    if (GAMESTATE == MAIN_MENU) set_leds(spiled_mem_base, LED_COLOR_OFF, LED_COLOR_OFF);
    menu_sel_idx = 0;
    break;
  case HIGHSCORES:
  case GAME_OVER:
    change_game_state(&GAMESTATE, MAIN_MENU);
    set_leds(spiled_mem_base, LED_COLOR_OFF, LED_COLOR_OFF);
    menu_sel_idx = 0;
    break;
  case GAME_RUNNING:
    change_game_state(&GAMESTATE, GAME_PAUSED);
    break;
  default:
    break;
  }
}

void update_menu_sel_idx() {
  int rotation_delta = knob_rotation_delta(KNOB_M, spiled_mem_base, menu_knob_rotation);
  menu_knob_rotation = knob_rotation(KNOB_M, spiled_mem_base);

  int menu_items_count = 0;
  switch (GAMESTATE) {
  case MAIN_MENU:
    menu_items_count = MAIN_MENU_ITEMS_COUNT;
    break;
  case GAMEMODE_MENU:
    menu_items_count = GAMEMODE_MENU_ITEMS_COUNT;
    break;
  case GAME_PAUSED:
    menu_items_count = PAUSE_MENU_ITEMS_COUNT;
    break;
  default:
    break;
  }

  if (rotation_delta > MENU_KNOB_THRESHOLD) menu_sel_idx = (menu_sel_idx + 1) % menu_items_count;
  if (rotation_delta < -MENU_KNOB_THRESHOLD) menu_sel_idx = (menu_sel_idx - 1 + menu_items_count) % menu_items_count;
}

void update_racket_pos() {
  if (GAMEMODE == PVP) {
    left_racket_pos = get_player_position(KNOB_L);
    right_racket_pos = get_player_position(KNOB_R);
  } else if (GAMEMODE == DEMO) {
    if (ball.dx > 0) {
      update_computer_racket_pos(&right_racket_pos, 0);
    } else {
      update_computer_racket_pos(&left_racket_pos, 0);
    }
  } else if (GAMEMODE == PVC) {
    left_racket_pos = get_player_position(KNOB_L);
    if (ball.dx > 0) {
      update_computer_racket_pos(&right_racket_pos, 20);
    }
  }

  if (left_racket_pos < 0) left_racket_pos = 0;
  if (left_racket_pos > LCD_H - RACKET_HEIGHT) left_racket_pos = LCD_H - RACKET_HEIGHT;
  if (right_racket_pos < 0) right_racket_pos = 0;
  if (right_racket_pos > LCD_H - RACKET_HEIGHT) right_racket_pos = LCD_H - RACKET_HEIGHT;
}

void update_computer_racket_pos(int *racket_pos, int error) {
  int ball_dist_x = (ball.dx > 0) ? LCD_W - ball.x - BALL_SIZE : ball.x;
  int predicted_y_no_error = ball.y + (ball.dy * ball_dist_x);
  int predicted_y = (error > 0) ? predicted_y_no_error + (rand() % (2 * error + 1) - error) : predicted_y_no_error;
  int target_pos = predicted_y - RACKET_HEIGHT / 2;
  *racket_pos += (target_pos - *racket_pos) / 5;
}

void update_ball_pos() {
  if (ball_hit_racket(KNOB_L, left_racket_pos) == TRUE) {
    if (DEBUG_ON) pprintf(PRINT_COLOR_GREEN, "Hit LEFT racket\n");
    ball.dx = -ball.dx;
    left_score++;
  } else if (ball_hit_racket(KNOB_R, right_racket_pos) == TRUE) {
    if (DEBUG_ON) pprintf(PRINT_COLOR_GREEN, "Hit RIGHT racket\n");
    ball.dx = -ball.dx;
    right_score++;
  } else if (ball_not_caught() == TRUE) {
    int score = ball.x < 0 ? right_score : left_score;
    update_high_scores(score);
    change_game_state(&GAMESTATE, GAME_OVER);
  }

  if (ball.y <= 0) {
    ball.y = 0;
    ball.dy = -ball.dy;
  } else if (ball.y + BALL_SIZE >= LCD_H) {
    ball.y = LCD_H - BALL_SIZE;
    ball.dy = -ball.dy;
  }
  ball.x += ball.dx;
  ball.y += ball.dy;
  last_collision++;
}

int get_player_position(Knob player) {
  int rotation = knob_rotation(player, spiled_mem_base);
  return (3 * rotation) % 256;
}

Bool ball_hit_racket(Knob player, int racket_y_pos) {
  int racket_x_pos = player == KNOB_L ? 0 : LCD_W - RACKET_WIDTH;

  Bool x_collision = FALSE;
  if (player == KNOB_L && (ball.x <= racket_x_pos + RACKET_WIDTH && ball.x + BALL_SIZE > racket_x_pos) &&
      last_collision > 20) { // LAST COLLISION - to prevent bugs at edge case of ball being hit by side of racket
    x_collision = TRUE;
    last_collision = 0;
  } else if (player == KNOB_R && (ball.x + BALL_SIZE >= racket_x_pos && ball.x < racket_x_pos + RACKET_WIDTH) && last_collision > 20) {
    x_collision = TRUE;
    last_collision = 0;
  }

  Bool y_collision = (ball.y + BALL_SIZE > racket_y_pos && ball.y < racket_y_pos + RACKET_HEIGHT);

  return x_collision && y_collision;
}

Bool ball_not_caught() { return (ball.x < 0 || ball.x + BALL_SIZE > LCD_W); }

void update_high_scores(int score) {
  for (int i = 0; i < MAX_HIGHSCORES; i++) {
    if (score > high_scores[i]) {
      for (int j = MAX_HIGHSCORES - 1; j > i; j--) {
        high_scores[j] = high_scores[j - 1];
      }
      high_scores[i] = score;
      break;
    }
  }
}

void reset_ball() {
  ball.x = LCD_W / 2;
  ball.y = LCD_H / 2;
  ball.dx = get_random_dir();
  ball.dy = get_random_dir();
}

void reset_scores() { left_score = right_score = 0; }

/* --------------------------------- RENDERING --------------------------------- */

void render_high_scores() {
  wipe_fb(fb);

  render_text_centered(20, "HIGH SCORES", LCD_COLOR_YELLOW, fb);

  for (int i = 0; i < MAX_HIGHSCORES; i++) {
    char score_text[30];
    snprintf(score_text, sizeof(score_text), "%d. %d", i + 1, high_scores[i]);
    render_text_centered(80 + (i * MENU_ITEM_HEIGHT), score_text, LCD_COLOR_WHITE, fb);
  }

  render_text_centered(LCD_H - 40, "Press button to go back", LCD_COLOR_WHITE, fb);

  write_lcd_fb(parlcd_mem_base, fb);
}

void render_game() {
  wipe_fb(fb);

  draw_racket(KNOB_L, left_racket_pos);
  draw_racket(KNOB_R, right_racket_pos);
  draw_ball();
  draw_score();
  draw_center_line();

  write_integer_bitmask(spiled_mem_base, (left_score + right_score));

  write_lcd_fb(parlcd_mem_base, fb);
}

void render_game_over() {
  wipe_fb(fb);

  render_text_centered(20, "GAME OVER", LCD_COLOR_YELLOW, fb);

  char winner_text[20];
  if (ball.x >= LCD_W - 1) {
    snprintf(winner_text, sizeof(winner_text), "Player 1 wins!");
  } else {
    snprintf(winner_text, sizeof(winner_text), "Player 2 wins!");
  }
  render_text_centered(LCD_H / 2, winner_text, LCD_COLOR_WHITE, fb);

  char final_score[20];
  snprintf(final_score, sizeof(final_score), "Score: %d - %d", left_score, right_score);
  render_text_centered(LCD_H / 2 + 30, final_score, LCD_COLOR_WHITE, fb);

  render_text_centered(LCD_H - 40, "Press button to continue", LCD_COLOR_WHITE, fb);

  write_lcd_fb(parlcd_mem_base, fb);
}

void draw_racket(Knob player, int pos) {
  int x_pos = player == KNOB_L ? 0 : LCD_W - RACKET_WIDTH;
  for (size_t dy = 0; dy < RACKET_HEIGHT; dy++) {
    int y_pos = pos + dy;
    for (size_t dx = 0; dx < RACKET_WIDTH; dx++) {
      fb[LCD_W * y_pos + x_pos + dx] = (player == KNOB_L) ? LCD_COLOR_RED : LCD_COLOR_BLUE;
    }
  }
}

void draw_ball() {
  for (size_t i = 0; i < BALL_SIZE; i++) {
    for (size_t j = 0; j < BALL_SIZE; j++) {
      int draw_x = ball.x + i;
      int draw_y = ball.y + j;
      if (draw_x >= 0 && draw_x < LCD_W && draw_y >= 0 && draw_y < LCD_H) {
        fb[LCD_W * draw_y + draw_x] = LCD_COLOR_WHITE;
      }
    }
  }
}

void draw_score() {
  char left_score_text[10];
  char right_score_text[10];

  snprintf(left_score_text, sizeof(left_score_text), "%d", left_score);
  snprintf(right_score_text, sizeof(right_score_text), "%d", right_score);

  render_text(LCD_W / 4, 20, left_score_text, LCD_COLOR_WHITE, fb);
  render_text(3 * LCD_W / 4, 20, right_score_text, LCD_COLOR_WHITE, fb);
}

void draw_center_line() {
  for (int y = 0; y < LCD_H; y += 8) {
    for (int i = 0; i < CENTER_LINE_WIDTH; i++) {
      fb[LCD_W * (y + i) + (LCD_W / 2)] = LCD_COLOR_GRAY;
    }
  }
}

void draw_goodbye_screen() {
  wipe_fb(fb);
  render_text_centered(60, "GOODBYE!", LCD_COLOR_WHITE, fb);

  for (int x = 0; x < LCD_W; x++) { // draw a thick line under 'goodbye'
    fb[77 * LCD_W + x] = LCD_COLOR_GRAY;
    fb[78 * LCD_W + x] = LCD_COLOR_GRAY;
  }

  write_led(spiled_mem_base, 0x0);
  write_lcd_fb(parlcd_mem_base, fb);
}
