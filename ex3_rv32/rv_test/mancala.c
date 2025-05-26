#include "Mancala.h"

#include "io.h"


// int main() {
// #if defined(NATIVE_MODE)
//     printf("do nothing...\n");
// #else
//     Mancala();
// #endif
//     return 0;
// }

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
