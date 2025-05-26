#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "io.h"


#define DICE 5
#define ROUNDS 5
#define TOTAL_ROLES 13


void roll_dice(int dice[DICE]);
int yacht_score(int dice[DICE], const char *role);
void uart_puts_id(int id, const char *s);
void uart_putint_id(int id, int n);
void uart_newline_id(int id);
void recv_choice(int id, char *buf, int maxlen);
int Yacht();
