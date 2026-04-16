#include "string.h"

// Inverte uma string (necessário para o itoa)
void reverse(char s[]) {
    int i, j;
    char c;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

// Retorna o tamanho da string
int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') i++;
    return i;
}

// Converte inteiro para ASCII (nossa itoa)
void itoa(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n; // Trata números negativos
    i = 0;
    do {
        str[i++] = n % 10 + '0'; // Pega o último dígito e converte p/ ASCII
    } while ((n /= 10) > 0);     // Remove o último dígito
    
    if (sign < 0) str[i++] = '-';
    str[i] = '\0';
    reverse(str); // Inverte para a ordem correta
}