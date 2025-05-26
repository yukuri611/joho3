#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "io.h"
#include "Mancala.h"
#include "Yacht.h"

int Nine_Mens_Morris(){return 0;}
int hit_and_blow(){return 0;}
int hit_and_blow_2(){return 0;}
int Speed(){return 0;}


#define DICE 5
#define ROUNDS 5
#define TOTAL_ROLES 13


unsigned int N1,N0,NN;
int int_to_gpio[10] = {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111};
const char *game_list[] = {"\0", "マンカラ", "ヨット", "スピード", "ナインメンズモリス", "ヒットアンドブロー", "ヒットアンドブロー(非対称)"};

void timer_handler();
void print_segment_int(int M);


int main(){
    char ch = 0;
    int game,result,i;
    int terminated=1;

    #if defined(NATIVE_MODE)
    printf("do nothing...\n");
    #else
    gpio->data = 0;
    for(i=0;i<2;i++){
        set_uart_ID(i);
        for(game=1;game<7;game++)
            printf("\n%d  %s",game, game_list[game]);
    }
    set_uart_ID(1);
    printf("\n---相手のゲーム選択を待っています...---\n");
    set_uart_ID(0);
    printf("\nゲームを選択してください: ");
    while(1){
        store_input(&ch, &terminated);
        if('1' <= ch && ch <= '5') {
            game = ch - '0';
            printf("\n");
            break;
        }
    }

    if(game == 1)       result = Mancala();
    else if(game == 2)  result = Yacht();
    else if(game == 3)  result = Speed();
    else if(game == 4)  result = Nine_Mens_Morris();
    else if(game == 5)  result = hit_and_blow();
    else if(game == 6)  result = hit_and_blow_2();

    if(game == 2){
        print_segment_int(result);
        gpio->dout7SEG[1] = N1;
        gpio->dout7SEG[0] = N0;
    }
    else if(game == 6){
        print_segment_int(result);
        if(result > 30){
            gpio->dout7SEG[1] = 0b01000110000111000101000001010100; // "turn"
            gpio->dout7SEG[0] = 0b01011100000111000111100101010000; // "over"
        }
        else{
            gpio->dout7SEG[1] = N0;
            gpio->dout7SEG[0] = 0b01000110000111000101000001010100; // "turn"
        }
    }
    else{
        if(result == 1){
            for(i=0;i<2;i++){
                set_uart_ID(i);
                printf("Winner is 1.\n");
            }
            N1 = 0b00000000000000000000011000000000; // "   1"
            N0 = 0b10111110001100000001000001010100; // " Win"
        }
        else if(result == 2){
            for(i=0;i<2;i++){
                set_uart_ID(i);
                printf("Winner is 2.\n");
            }
            N1 = 0b00000000000000000101101100000000; // "   2"
            N0 = 0b10111110001100000001000001010100; // " Win"
        }
        else if(result == 0){
            for(i=0;i<2;i++){
                set_uart_ID(i);
                printf("draw.\n");
            }
            N1 = 0b00000000000000000000000001011110; // "   d"
            N0 = 0b01010000010111001001110000010000; // "ro w"
        }
        else{
            for(i=0;i<2;i++){
                set_uart_ID(i);
                printf("error.\n");
            }
            N1 = 0b00000000000000000000000001111001; // "   E"
            N0 = 0b01010000010100000101110001010000; // "rror"
        }
        set_mtimer_interval(300/*msec*/);
        timer_interrupt_hook = timer_handler;
        enable_timer_interrupt(); //割り込みハンドラを設定するための関数
    }
    set_uart_ID(0);
    printf("\npress any key to end your program...\n");
    io_getch();
    for(i=0;i<2;i++){
        set_uart_ID(i);
        printf("\nThank you for playing!\n");
    }
    asm volatile("csrc mie, %[new]" : : [new] "r" (0x00000080));    /// clear meie[7] : machine-external-interrupt-enable

    #endif
    return 0;
}


#if !defined(NATIVE_MODE)
void timer_handler(){
    NN = (N1 << 8) | (N0 >> 24);
    N0 = (N0 << 8) | (N1 >> 24);
    N1 = NN;
    gpio->dout7SEG[1] = N1;
    gpio->dout7SEG[0] = N0;
}
#endif

void print_segment_int(int M){
    int j, k;
    N0 = N1 = 0;
    if (M == 0) N1 = int_to_gpio[0];
    else {
        k = 1;
        while(k <= M) k *= 10;
        while(k != 1){
            k /= 10;
            j = M / k;
            if(k >= 10000)  N0 = (N0 << 8) & int_to_gpio[j];
            else            N1 = (N1 << 8) & int_to_gpio[j];
            M -= k * j;
        }
    }
}


//Mancala

int Mancala(void) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    int board[PITS+1];
    init_board(board);

    int player    = 1;   // 1 = プレイヤー1, 2 = プレイヤー2
    int uart_flag = 0;   // 0 → TeraTerm0, 1 → TeraTerm1 (TeraTerm0がplayer1, TeraTerm1がplayer2)
    char buf[BUF_SIZE];
    int terminated = 0;

    set_uart_ID(uart_flag);
    int onGame = 1;
    for (int id = 0; id <= 1; id++) {
        set_uart_ID(id);
        printf("\n\nゲームスタート\n\n");
    }
    while (onGame) {
        // 最新盤面を双方端末に表示
        for (int id = 0; id <= 1; id++) {
            set_uart_ID(id);
            printf("\n--- 盤面が更新されました ---\n");
            print_board(board, id + 1, player);
        }

        // 手番側端末に切り替え
        uart_flag = (player == 1 ? 0 : 1);
        set_uart_ID(uart_flag);

        const char* color = (player == 1 ? ANSI_PLAYER1 : ANSI_PLAYER2);
        int choice, pit;
        // 入力受信
        while (!terminated){
            printf("%s>>> Player %d の手番です。ポケット1～6を選択してください：%s",
               color, player, ANSI_RESET);
            for (int i = 0; i < BUF_SIZE; i++) buf[i] = '\0';
            store_input(buf, &terminated);
            
            if (terminated) break;

            choice = atoi(buf);
            if (choice < 1 || choice > 6) {
                printf("  (エラー) 1～6 の整数を送信してください。\n");
                continue;
            }

            pit = (player == 1 ? choice : choice + 7);
            if (board[pit] == 0) {
                printf("  (エラー) ポケット %d は空です。\n", choice);
                continue;
            }
            break;
        }
        if (terminated) break;
        

        // 石を配分
        int stones = board[pit];
        board[pit] = 0;
        int pos = pit;
        while (stones > 0) {
            pos = pos % PITS + 1;
            if ((player == 1 && pos == 14) || (player == 2 && pos == 7)) continue;
            board[pos]++;
            stones--;
        }

        if (is_game_over(board)) {
            break;
        }

        // 自ゴール着地なら連続手番。違うなら相手の番に移行
        int in_goal = (player == 1 && pos == 7) || (player == 2 && pos == 14);
        if (in_goal) printf("%s%s\n--- 最後の石がポケットに入りました！ ---\n%s", ANSI_BOLD, ANSI_POCKET, ANSI_RESET);
        else {
            player = 3 - player;
            printf("\n--- 入力が処理されました。相手の入力を待っています... ---\n");
        }
    }

    // 最終結果表示
    int s1=0,s2=0;
    for(int i=1;i<=6;i++) s1+=board[i];
    for(int i=8;i<=13;i++) s2+=board[i];
    int res;
    for (int id = 0; id <= 1; id++) {
        set_uart_ID(id);
        printf("ゲーム終了\n");
        print_board(board, id + 1, 0);
    }
    if (s1 == 0) {
        res = 1;
    }
    else if (s2 == 0) {
        res = 2;
    }
    else {
        res = 0;
    }
    set_uart_ID(0);
    return res;
#endif
    return 0;
}

void init_board(int board[]) {
    for (int i = 1; i <= PITS; i++) board[i] = (i==7 || i==14)?0:4;
    for (int i = 1; i <= 5; i++) board[i] = 0;
    board[6] = 1;
}

void print_board(int b[], int perspective, int current_player) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    // 上段
    printf("\n");
    if (perspective == 1) {
        printf("%s player2側\n%s", ANSI_PLAYER2, ANSI_RESET);
    }
    else {
        printf("%s player1側\n%s", ANSI_PLAYER1, ANSI_RESET);
    }
    printf("    │"); for(int i=0;i<6;i++){int idx=(perspective==1?13-i:6-i); printf(" %2d  │",b[idx]);} printf("\n");

    // ゴール行
    int lg=(perspective==1?b[14]:b[7]), rg=(perspective==1?b[7]:b[14]);
    printf(" %2d ",lg); printf("─────────────────────────────────────"); printf(" %2d  ",rg); printf("\n");

    // 下段
    printf("    │"); for(int i=0;i<6;i++){int idx=(perspective==1?i+1:i+8); printf(" %2d  │",b[idx]);} printf("\n");
    if (perspective == 1) {
        printf("%s player1側\n%s", ANSI_PLAYER1, ANSI_RESET);
    }
    else {
        printf("%s player2側\n%s", ANSI_PLAYER2, ANSI_RESET);
    }
    printf("\n");
#endif
}

int is_game_over(int b[]) {
    int sa=0,sb=0;
    for(int i=1;i<=6;i++) sa+=b[i];
    for(int i=8;i<=13;i++) sb+=b[i];
    return (sa==0||sb==0);
}

void store_input(char* store, int* terminated) {
#if defined(NATIVE_MODE)
    (void)store;(void)terminated;
#else
    int pos=0;
    while(1){ 
        char ch=io_getch();
        if(ch==BACK_SPACE_CODE){
            if(pos>0){
                pos--;store[pos]='\0';
            } 
            continue; 
        }
        if(ch==TERMINATE_CODE){
            store[pos]='\0';
            printf("\nCtrl-Z detected...\n");
            *terminated=1; break; 
        }
        store[pos++]=ch;
        if(ch=='\n'||pos==BUF_SIZE-1){
            store[pos]='\0';
            if(ch!='\n') io_putch('\n');
            break;
        }
    }
#endif
}


//Yacht
const char *roles[TOTAL_ROLES] = {
    "Aces", "Deuces", "Treys", "Fours", "Fives", "Sixes",
    "Choice", "Four-of-a-kind", "Full-house", "S.Straight", "B.Straight", "Yacht"
};


void roll_dice(char dice[DICE]) {
#if defined(NATIVE_MODE)
    for(char i=0;i<DICE;i++) dice[i] = rand()%6+1;
#else
    for(char i=0;i<DICE;i++) dice[i] = rand()%6+1;
#endif
}


void uart_puts(const char *s) {
#if defined(NATIVE_MODE)
    printf("%s", s);
#else
    set_uart_ID(0);
    while(*s) _UARTBuf_putch(*s++);
#endif
}


void uart_putint(int n) {
#if defined(NATIVE_MODE)
    printf("%d", n);
#else
    char tmp[12], *p = tmp + 11;
    int neg = 0;
    *p = 0;
    if (n == 0) { _UARTBuf_putch('0'); return; }
    if (n < 0) { neg = 1; n = -n; }
    while (n > 0) { *(--p) = '0' + (n % 10); n /= 10; }
    if (neg) *(--p) = '-';
    while (*p) _UARTBuf_putch(*p++);
#endif
}


void print_dice(const char dice[DICE]) {
#if defined(NATIVE_MODE)
    printf("Your dice: [%d] [%d] [%d] [%d] [%d]\n", dice[0], dice[1], dice[2], dice[3], dice[4]);
#else
    uart_puts("Your dice: [");
    for(char i=0;i<DICE;i++) {
        uart_putint(dice[i]);
        if(i<DICE-1) uart_puts("] [");
    }
    uart_puts("]\n");
#endif
}


void send_str(const char *s) { uart_puts(s); }


void recv_str(char *buf, char max_len) {
#if defined(NATIVE_MODE)
    fgets(buf, max_len, stdin);
    buf[strcspn(buf, "\n")] = 0;
#else
    set_uart_ID(0);
    char pos = 0;
    while(1) {
        char ch = _UARTBuf_getch();
        if(ch=='\n'||pos>=max_len-1) break;
        buf[pos++] = ch;
    }
    buf[pos]=0;
#endif
}


void send_int(int val) {
#if defined(NATIVE_MODE)
    printf("%d\n", val);
#else
    uart_putint(val); _UARTBuf_putch('\n');
#endif
}


int recv_int() {
#if defined(NATIVE_MODE)
    int v;
    scanf("%d", &v);
    getchar();
    return v;
#else
    set_uart_ID(0);
    char buf[8]; char pos = 0;
    while (1) {
        char ch = _UARTBuf_getch();
        if (ch == '\n' || pos >= 7) break;
        buf[pos++] = ch;
    }
    buf[pos] = 0;
    return atoi(buf);
#endif
}


int yacht_score(const char dice[DICE], const char *chosen_role) {
    char counts[7] = {0};
    char i, max_count = 0, pairs = 0;
    int sum = 0, score = 0;
    for(i=0;i<DICE;i++) {
        counts[dice[i]]++;
        sum += dice[i];
        if(counts[dice[i]] > max_count) max_count = counts[dice[i]];
    }
    for(i=1;i<=6;i++) if(counts[i]==2) pairs++;


    if(strcmp(chosen_role, "Aces") == 0) score = counts[1] * 1;
    else if(strcmp(chosen_role, "Deuces") == 0) score = counts[2] * 2;
    else if(strcmp(chosen_role, "Treys") == 0) score = counts[3] * 3;
    else if(strcmp(chosen_role, "Fours") == 0) score = counts[4] * 4;
    else if(strcmp(chosen_role, "Fives") == 0) score = counts[5] * 5;
    else if(strcmp(chosen_role, "Sixes") == 0) score = counts[6] * 6;
    else if(strcmp(chosen_role, "Choice") == 0) score = sum;
    else if(strcmp(chosen_role, "Four-of-a-kind") == 0 && max_count >= 4) score = sum;
    else if(strcmp(chosen_role, "Full-house") == 0 && max_count == 3 && pairs == 1) score = sum;
    else if(strcmp(chosen_role, "S.Straight") == 0) {
        for(i=1;i<=3;i++) {
            if(counts[i]&&counts[i+1]&&counts[i+2]&&counts[i+3])
                score = 15;
        }
    }
    else if(strcmp(chosen_role, "B.Straight") == 0) {
        if((counts[1]&&counts[2]&&counts[3]&&counts[4]&&counts[5]) ||
           (counts[2]&&counts[3]&&counts[4]&&counts[5]&&counts[6]))
            score = 30;
    }
    else if(strcmp(chosen_role, "Yacht") == 0 && max_count == 5) score = 50;
    return score;
}
int Yacht() {
#if defined(NATIVE_MODE)
    printf("Native");
#else
    enable_UART_interrupt();
    srand(sysctrl->mtime);


    char dice[DICE], used_roles[TOTAL_ROLES] = {0};
    char chosen_role[16];
    int total_score = 0, upper_score = 0;


    for(char round=0; round<ROUNDS; round++) {
        send_str("\n--- Round ");
        uart_putint(round + 1);
        send_str(" ---\n");
        roll_dice(dice);
        print_dice(dice);


        send_str("Available roles:\n");
        for(char i=0;i<TOTAL_ROLES;i++)
            if(!used_roles[i]) { send_str(roles[i]); send_str("\n"); }


        send_str("Choose a role:\n");
        recv_str(chosen_role, sizeof(chosen_role));


        char valid = 0;
        for(char i=0;i<TOTAL_ROLES;i++) {
            if(!used_roles[i] && strcmp(chosen_role, roles[i]) == 0) {
                valid = 1; used_roles[i] = 1; break;
            }
        }
        if(!valid) {
            send_str("Invalid or already used role. Please select again.\n");
            round--;
            continue;
        }
        int round_score = yacht_score(dice, chosen_role);
        total_score += round_score;
        if(strcmp(chosen_role, "Aces") == 0 || strcmp(chosen_role, "Deuces") == 0 || strcmp(chosen_role, "Treys") == 0 ||
           strcmp(chosen_role, "Fours") == 0 || strcmp(chosen_role, "Fives") == 0 || strcmp(chosen_role, "Sixes") == 0)
            upper_score += round_score;


        send_str("Your score for ");
        send_str(chosen_role);
        send_str(": ");
        uart_putint(round_score);
        send_str("\n");
    }


    if(upper_score >= 63) {
        send_str("Upper section bonus: 35 points!\n");
        total_score += 35;
    }
    send_str("\nGame Over! Your total score: ");
    uart_putint(total_score);
    send_str("\n");
    return total_score;
    #endif
}


