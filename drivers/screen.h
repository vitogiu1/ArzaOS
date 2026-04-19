#ifndef SCREEN_H
#define SCREEN_H

// Variáveis públicas
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0F
#define GREEN_ON_BLACK 0x0A
#define RED_ON_BLACK 0x04
#define YELLOW_ON_BLACK 0x0E
#define BLUE_ON_BLACK 0x0B
#define GRAY_ON_BLACK 0x08
#define RED_WHITE 0X0C

void clear_screen();
void print(char* message, char color);
int get_cursor_offset();
void set_cursor_offset(int offset);
void print_backspace();

#endif