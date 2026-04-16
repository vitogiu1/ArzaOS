#ifndef SCREEN_H
#define SCREEN_H

//Nossas variáveis públicas

#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0F
#define GREEN_ON_BLACK 0x0A
#define RED_ON_BLACK 0x04
#define YELLOW_ON_BLACK 0x0E

// Apenas avisamos que essas funções existem para quem quiser importar o screen.h
void clear_screen();
void print(char* message, char color);

#endif