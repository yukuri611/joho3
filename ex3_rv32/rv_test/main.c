#include "GameHeaders.h"


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
        if('1' <= ch && ch <= '6') {
            game = ch - '0';
            printf("\n");
            break;
        }
    }


    switch(game) {
        case 1:
            result = Mancala();
            break;
        case 2:
            result = Yacht();
            break;
        case 3:
            result = Speed();
            break;
        case 4:
            result = Nine_Mens_Morris();
            break;
        case 5:
            result = Hit_and_Blow();
            break;
        case 6:
            result = Hit_and_Blow_2();
            break;
        default:
            // 必要に応じてデフォルトの処理をここに追加
            break;
    }

    if(game == 6){
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
            if(k >= 10000)  N1 = (N1 << 8) | int_to_gpio[j];
            else            N0 = (N0 << 8) | int_to_gpio[j];
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
        printf("\n\nゲームスタート\nゲーム名: マンカラ\n\n");
        printf("ゲームの説明 --------------------------------------------------------\n");
        printf("下に表示されているボードについて説明します。\n上側が相手のポケット。下側が自分側のポケットです。\n各ポケットには番号が割り振られており、左から順に1から6になっています。\n自分のターンになったら、ポケットの番号を指定してください。\n自分側のポケットの石をすべて0にしたら勝利です！\nより詳細な内容は、レポートのゲーム内容について記載した部分を参照してください。\n");
        printf("--------------------------------------------------------------------\n");
    }
    while (onGame) {
        // 最新盤面を双方端末に表示
        for (int id = 0; id <= 1; id++) {
            set_uart_ID(id);
            printf("\n--- 盤面が更新されました ---\n");
            print_board(board, id + 1, player);
        }
        uart_flag = (player == 1 ? 1 : 0);
        set_uart_ID(uart_flag);
        printf("相手の入力を待っています。\n");

        // 手番側端末に切り替え
        uart_flag = (player == 1 ? 0 : 1);
        set_uart_ID(uart_flag);

        const char* color = (player == 1 ? ANSI_PLAYER1 : ANSI_PLAYER2);
        int choice, pit;
        // 入力受信
        while (!terminated){
            printf("%s>>> あなた (Player %d) の手番です。ポケットの番号(1から6)を選択してください：%s",
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




// Roll DICE dice into array
void roll_dice(int dice[DICE]) {
    #if defined(NATIVE_MODE)
    printf("Native");
 #else
    for (int i = 0; i < DICE; ++i) {
        dice[i] = rand() % 6 + 1;
    }
    #endif
}




// Compute yacht score for given dice and role
int yacht_score(int dice[DICE], const char *role) {
    #if defined(NATIVE_MODE)
    printf("Native");
#else
    int cnt[7] = {0}, sum = 0, maxc = 0, pairs = 0;
    for (int i = 0; i < DICE; ++i) {
        int v = dice[i]; cnt[v]++; sum += v;
        if (cnt[v] > maxc) maxc = cnt[v];
    }
    for (int v = 1; v <= 6; ++v) if (cnt[v] == 2) pairs++;
    if (!strcmp(role, "Aces")) return cnt[1];
    if (!strcmp(role, "Deuces")) return cnt[2]*2;
    if (!strcmp(role, "Treys")) return cnt[3]*3;
    if (!strcmp(role, "Fours")) return cnt[4]*4;
    if (!strcmp(role, "Fives")) return cnt[5]*5;
    if (!strcmp(role, "Sixes")) return cnt[6]*6;
    if (!strcmp(role, "Choice")) return sum;
    if (!strcmp(role, "Four-of-a-kind") && maxc>=4) return sum;
    if (!strcmp(role, "Full-house") && maxc==3 && pairs==1) return sum;
    if (!strcmp(role, "S.Straight")) {
        for (int s=1; s<=3; ++s)
            if (cnt[s]&&cnt[s+1]&&cnt[s+2]&&cnt[s+3]) return 15;
    }
    if (!strcmp(role, "B.Straight")) {
        if ((cnt[1]&&cnt[2]&&cnt[3]&&cnt[4]&&cnt[5]) ||
            (cnt[2]&&cnt[3]&&cnt[4]&&cnt[5]&&cnt[6])) return 30;
    }
    if (!strcmp(role, "Yacht") && maxc==5) return 50;
    return 0;
#endif
}




// UART output helpers for specific channel
void uart_puts_id(int id, const char *s) {
    #if defined(NATIVE_MODE)
    printf("Native");
#else
    for (int ch = 0; s[ch]; ++ch) {
        set_uart_ID(id);
        _UARTBuf_putch(s[ch]);
    }
#endif    
}
void uart_putint_id(int id, int n) {
#if defined(NATIVE_MODE)
    printf("Native");
#else    
    char buf[12]; char *p = buf + sizeof(buf) - 1;
    *p = '\0';
    if (n == 0) {
        set_uart_ID(id); _UARTBuf_putch('0');
        return;
    }
    int neg = n < 0;
    if (neg) n = -n;
    while (n) { *--p = '0' + (n % 10); n /= 10; }
    if (neg) *--p = '-';
    for (; *p; ++p) { set_uart_ID(id); _UARTBuf_putch(*p); }
#endif
}
void uart_newline_id(int id) {
    #if defined(NATIVE_MODE)
    printf("Native");
#else
    set_uart_ID(id); _UARTBuf_putch('\n');
#endif    
}




// Send prompt and receive string
void recv_choice(int id, char *buf, int maxlen) {
    #if defined(NATIVE_MODE)
    printf("Native");
#else
    set_uart_ID(id);
    int pos = 0;
    while (pos < maxlen - 1) {
        char c = _UARTBuf_getch();
        if (c == '\n') break;
        buf[pos++] = c;
    }
    buf[pos] = '\0';
    UARTBuf_consume_new_line(&UART_BUF_RX[id]);
#endif
}




int Yacht() {
    #if defined(NATIVE_MODE)
    printf("Native");
#else
    enable_UART_interrupt();
    srand(sysctrl->mtime);
    int total[2] = {0, 0};
    int dice0[DICE], dice1[DICE];
    char choice0[16], choice1[16];
    for (int round = 1; round <= ROUNDS; ++round) {
        // Player 0 turn
        uart_puts_id(0, "\n--- Round "); uart_putint_id(0, round); uart_puts_id(0, " ---\n");
        roll_dice(dice0);
        uart_puts_id(0, "Your dice: ");
        for (int i = 0; i < DICE; ++i) {
            uart_putint_id(0, dice0[i]); uart_puts_id(0, " ");
        }
        uart_puts_id(0, "\nAvailable roles:\n");
        for (int i = 0; i < TOTAL_ROLES; ++i) {
            uart_puts_id(0, roles[i]); uart_puts_id(0, "\n");
        }
        uart_puts_id(0, "Choose role:\n");
        recv_choice(0, choice0, sizeof(choice0));
        int sc0 = yacht_score(dice0, choice0);
        total[0] += sc0;
        uart_puts_id(0, "Score: "); uart_putint_id(0, sc0); uart_newline_id(0);




        // Player 1 turn
        uart_puts_id(1, "\n--- Round "); uart_putint_id(1, round); uart_puts_id(1, " ---\n");
        roll_dice(dice1);
        uart_puts_id(1, "Your dice: ");
        for (int i = 0; i < DICE; ++i) {
            uart_putint_id(1, dice1[i]); uart_puts_id(1, " ");
        }
        uart_puts_id(1, "\nAvailable roles:\n");
        for (int i = 0; i < TOTAL_ROLES; ++i) {
            uart_puts_id(1, roles[i]); uart_puts_id(1, "\n");
        }
        uart_puts_id(1, "Choose role:\n");
        recv_choice(1, choice1, sizeof(choice1));
        int sc1 = yacht_score(dice1, choice1);
        total[1] += sc1;
        uart_puts_id(1, "Score: "); uart_putint_id(1, sc1); uart_newline_id(1);
    }
    // Show final results
    for (int id = 0; id < 2; ++id) {
        int opp = total[1-id];
        uart_puts_id(id, "\nFinal Your total: "); uart_putint_id(id, total[id]);
        uart_puts_id(id, " Opponent total: "); uart_putint_id(id, opp);
        uart_puts_id(id, "\n");
        if (total[id] > opp) {
            return 1;
        }
        else if (total[id] < opp){
            return 2;
        }
        else {
            return 0;
        }
    }
#endif
 return 0;
}


//Speed
const char ranks[] = "A23456789TJQK";




int card_to_point(char card) {
    if(card=='A') return 1;
    else if(card>='2' && card<='9') return card-'0';
    else if(card=='T') return 10;
    else if(card=='J') return 11;
    else if(card=='Q') return 12;
    else if(card=='K') return 13;
    return 0;
}


void dual_uart_puts(const char *s) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else    
    set_uart_ID(0); for(const char *p=s;*p;p++) _UARTBuf_putch(*p);
    set_uart_ID(1); for(const char *p=s;*p;p++) _UARTBuf_putch(*p);
#endif    
}


void dual_uart_putch(char c) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else    
    set_uart_ID(0);
    _UARTBuf_putch(c);
    set_uart_ID(1);
    _UARTBuf_putch(c);
#endif
}


int uart_has_input(int pid) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    return UART_BUF_RX[pid].not_empty;
#endif    
}


char uart_receive(int pid) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else    
    set_uart_ID(pid);
    return _UARTBuf_getch();
#endif    
}


void uart_clear_buf(int pid) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else    
    set_uart_ID(pid);
    while(UART_BUF_RX[pid].not_empty) _UARTBuf_getch();
#endif    
}


int Speed() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else    
    enable_UART_interrupt();
    srand((unsigned)sysctrl->mtime);


    int scores[2] = {0,0};


    dual_uart_puts("=== トランプ早打ちゲーム（10ラウンド） ===\n");
    dual_uart_puts("最初に指定カードを打ったプレイヤーに得点を与えます。\n\n");


    for(int round=1; round<=10; round++) {
        char target_card = ranks[rand()%13];
       
        dual_uart_puts("\n--- ラウンド ");
        dual_uart_putch('0'+(round/10));
        dual_uart_putch('0'+(round%10));
        dual_uart_puts(" ---\n");


        dual_uart_puts("指定カード: ");
        dual_uart_putch(target_card);
        dual_uart_puts("\nスタート!\n");


        int winner = -1;
        char input;


        while (winner==-1) {
            for (int pid=0; pid<2; pid++) {
                if(uart_has_input(pid)) {
                    input = uart_receive(pid);
                    if(input=='\n' || input=='\r') continue;


                    dual_uart_puts("Player ");
                    dual_uart_putch(pid?'2':'1');
                    dual_uart_puts(" → ");
                    dual_uart_putch(input);
                    dual_uart_puts(": ");


                    if(input==target_card) {
                        int points=card_to_point(target_card);
                        scores[pid]+=points;
                        dual_uart_puts("正解！ +");
                        dual_uart_putch('0'+(points/10));
                        dual_uart_putch('0'+(points%10));
                        dual_uart_puts("ポイント\n");
                        winner=pid;
                        break;
                    } else {
                        dual_uart_puts("不正解\n");
                    }
                }
            }
        }


        uart_clear_buf(0);
        uart_clear_buf(1);
    }


    dual_uart_puts("\n=== 最終結果 ===\n");
    dual_uart_puts("Player1 得点: ");
    dual_uart_putch('0'+(scores[0]/10));
    dual_uart_putch('0'+(scores[0]%10));
    dual_uart_puts("\n");


    dual_uart_puts("Player2 得点: ");
    dual_uart_putch('0'+(scores[1]/10));
    dual_uart_putch('0'+(scores[1]%10));
    dual_uart_puts("\n");


    dual_uart_puts("\n🎉 勝者は ");
    if(scores[0]>scores[1]) dual_uart_puts("Player1");
    else if(scores[1]>scores[0]) dual_uart_puts("Player2");
    else dual_uart_puts("引き分け");
    dual_uart_puts(" です！🎉\n");


    dual_uart_puts("ゲーム終了です。\n");
    if(scores[0]>scores[1]){return 1;}
else if(scores[0]<scores[1]){return 2;}
else{return 0;}
#endif
}

//Nine Mens Morris
int Nine_Mens_Morris(){
    int i,j,k,_k,dis,_dis,lank,file,min,max,pass_flag,adjacent_flag,mills,number_ntp,turn;
    char not_player,player,COPY[7][7],ch;
    char BOARD[7][7] = {
            {'X', '_', '_', 'X', '_', '_', 'X'},
            {'_', 'X', '_', 'X', '_', 'X', '_'},
            {'_', '_', 'X', 'X', 'X', '_', '_'},
            {'X', 'X', 'X', '_', 'X', 'X', 'X'},
            {'_', '_', 'X', 'X', 'X', '_', '_'},
            {'_', 'X', '_', 'X', '_', 'X', '_'},
            {'X', '_', '_', 'X', '_', '_', 'X'}};


    srand((unsigned int) time(NULL));


    #if defined(NATIVE_MODE)
    printf("do nothing...\n");
    #else
    gpio->data = 0;
    turn = 0;
    pass_flag = 0;
    while(1){
        turn++;
        player = turn % 2 == 1 ? '1' : '2';
        not_player = turn % 2 == 0 ? '1' : '2';


        // print board;
        for(j=0;j<2;j++){
            set_uart_ID(j);
            for(i=6;i>=0;i--){
                if(i==0 || i==6)        printf("%d %c-----------%c-----------%c \n",i+1,BOARD[i][0],BOARD[i][3],BOARD[i][6]);
                else if(i==1 || i==5)   printf("%d |   %c-------%c-------%c   | \n",i+1,BOARD[i][1],BOARD[i][3],BOARD[i][5]);
                else if(i==2 || i==4)   printf("%d |   |   %c---%c---%c   |   | \n",i+1,BOARD[i][2],BOARD[i][3],BOARD[i][4]);
                else if(i==3)           printf("%d %c---%c---%c       %c---%c---%c \n",i+1,BOARD[i][0],BOARD[i][1],BOARD[i][2],BOARD[i][4],BOARD[i][5],BOARD[i][6]);
                if(i==1 || i==6)        printf("  |           |           | \n");
                else if(i==2 || i==5)   printf("  |   |       |       |   | \n");
                else if(i==3 || i==4)   printf("  |   |   |       |   |   | \n");
            }
            for(i=0;i<7;i++) printf("  %c ",'a'+i);
            printf("\n");
            printf("プレイヤー %c の第 %d ターンです\n",player,(turn+1)/2,turn);
        }
        set_uart_ID(not_player-'1');
        printf("--- 相手の入力を待っています... ---\n");


        if(turn > 10){
             number_ntp = 0;
                for(i=0;i<7;i++)
                    for(j=0;j<7;j++)
                        if(BOARD[i][j] == player)
                            number_ntp++;
            if(number_ntp < 3)
                return 1 - turn % 2;
        }


        if(turn<11){
            set_uart_ID(player-'1');
            while(1){
                printf("駒を置く場所を選択してください  : ");
                fflush(stdout);
                file = io_getch() - 'a';
                lank = io_getch() - '1';
                ch = io_getch();
                if(ch != '\n') printf("\n");
                if(-1 < lank && lank < 7 && -1 < file && file < 7 &&  BOARD[lank][file] == 'X'){
                    BOARD[lank][file] = player;
                    break;
                }
                fflush(stdout);
            }
            for(i=0;i<2;i++){
                set_uart_ID(i);
                printf("%c%d に %c が置かれました\n",file+'a',lank+1,player);
            }
        }
        else{
            // 動くことができるかの判定
            pass_flag = 1;
            for(i=0;i<8;i++){
                _dis = i < 6 ? 3 - i / 2 : 0;
                 dis = i < 6 ? 0 : 1;
                 k   = i < 6 ? i / 2 : i - 6;
                _k   = i < 6 ? i / 2 + (i % 2) * (3 - i / 2) : 3;
                if((BOARD[k][_k] == player && BOARD[k+dis][_k+_dis] == 'X') || (BOARD[k][_k] == 'X' && BOARD[k+dis][_k+_dis] == player))                        pass_flag = 0;
                else if((BOARD[_k][k] == player && BOARD[_k+_dis][k+dis] == 'X') || (BOARD[_k][k] == 'X' && BOARD[_k+_dis][k+dis] == player))                   pass_flag = 0;
                else if((BOARD[6-k][6-_k] == player && BOARD[6-k-dis][6-_k-_dis] == 'X') || (BOARD[6-k][6-_k] == 'X' && BOARD[6-k-dis][6-_k-_dis] == player))   pass_flag = 0;
                else if((BOARD[6-_k][6-k] == player && BOARD[6-_k-_dis][6-k-dis] == 'X') || (BOARD[6-_k][6-k] == 'X' && BOARD[6-_k-_dis][6-k-dis] == player))   pass_flag = 0;
            }
            if(pass_flag == 1){
                for(i=0;i<2;i++){
                    set_uart_ID(i);
                    printf("%c は駒を動かすことができません\n",player);
                }
                continue;
            }


            while(1){
                set_uart_ID(player-'1');
                printf("動かす駒を選択してください : ");
                fflush(stdout);
                file = io_getch() - 'a';
                lank = io_getch() - '1';
                if(lank < 0 || 6 < lank || file < 0 || 6 < file || BOARD[lank][file] != player){
                    printf("\n");
                    continue;
                }
                printf(" -> ");
                fflush(stdout);
                _k = io_getch() - 'a';
                 k = io_getch() - '1';
                ch = io_getch();
                if(ch != '\n') printf("\n");


                // fromとtoで入力された場所が隣接しているかどうかの判定
                adjacent_flag = 0;
                if(-1 < k && k < 7 && -1 < _k && file < 7 && BOARD[k][_k] == 'X' && (lank == k || file == _k)){
                    if(lank == k){
                        min = file < _k ? file : _k;
                        max = file > _k ? file : _k;
                        j = k;
                    }
                    else if(file == _k){
                        min = lank < k ? lank : k;
                        max = lank > k ? lank : k;
                        j = _k;
                    }
                    if((j == 0 || j == 6) && max == min+3 && (min == 0 || min == 3))    adjacent_flag = 1;
                    if((j == 1 || j == 5) && max == min+2 && (min == 1 || min == 3))    adjacent_flag = 1;
                    if((j == 2 || j == 4) && max == min+1 && (min == 2 || min == 3))    adjacent_flag = 1;
                    if( j == 3            && max == min+1 &&  min != 3)                 adjacent_flag = 1;
                }


                if(adjacent_flag == 1){
                    BOARD[lank][file] = 'X';
                    BOARD[k][_k] = player;
                    break;
                }
            }
            for(i=0;i<2;i++){
                set_uart_ID(i);
                printf("%c%d の%c が %c%d に移動しました\n",file+'a',lank+1,player,k+'a',_k+1);
            }
            lank = k;
            file = _k;


        }


        // 消すかどうかを判定し、判定次第で消す
        for(i=0;i<7;i++)
            for(j=0;j<7;j++)
                COPY[i][j] = BOARD[i][j];
        mills = 0;
        for(i=0;i<8;i++){
            k   = i==3         ? 1 : (i==7 ? 5 : 3);
            dis = i==3 || i==7 ? 1 : (i<3 ? 3-i : i-3);
            j   = i==7         ? 3 : i;
            if(BOARD[j][k-dis] ==  BOARD[j][k] && BOARD[j][k] == BOARD[j][k+dis]){
                if(BOARD[j][k] == not_player)
                    COPY[j][k-dis] = COPY[j][k] = COPY[j][k+dis] = 'X';
                else if(BOARD[j][k] == player && lank == j){
                    if((j == 0 || j == 6) && file % 3 == 0)                 mills = 1;
                    else if((j == 1 || j == 5) && file % 2 == 1)            mills = 1;
                    else if((j == 2 || j == 4) && 2 <= file && file <= 4)   mills = 1;
                    else if(i == 3 && file <= 2)                            mills = 1;
                    else if(i == 7 && 4 <= file)                            mills = 1;
                }
            }
            if(BOARD[k-dis][j] == BOARD[k][j] && BOARD[k][j] == BOARD[k+dis][j]){
             if(BOARD[k][j] == not_player)
                 COPY[k-dis][j] = COPY[k][j] = COPY[k+dis][j] = 'X';
                if(BOARD[k][j] == player && file == j){
                    if((j== 0 || j== 6) && lank % 3 == 0)                   mills = 1;
                    else if((j == 1 || j == 5) && lank % 2 == 1)            mills = 1;
                    else if((j == 2 || j == 4) && 2 <= lank && lank <= 4)   mills = 1;
                    else if(i == 3 && lank <= 2)                            mills = 1;
                    else if(i == 7 && 4 <= lank)                            mills = 1;
                }
            }
        }
        if(mills == 1){
            // ターンが回っていない方のplayerの、mills状態でない駒の総数
            number_ntp = 0;
            for(i=0;i<7;i++)
                for(j=0;j<7;j++)
                    if(COPY[i][j] == not_player)
                        number_ntp++;
            if(number_ntp == 0){ // そんなものはないとき
                for(i=0;i<7;i++){
                    for(j=0;j<7;j++){
                        COPY[i][j] = BOARD[i][j];
                        if(BOARD[i][j] == not_player)
                            number_ntp++;
                    }
                }
            }
            number_ntp = rand() % number_ntp + 1; // 消す駒の番号
            for(i=0;i<7;i++){
                for(j=0;j<7;j++){
                    if(COPY[i][j] == not_player)
                        number_ntp--;
                    if(number_ntp == 0){
                        BOARD[i][j] = 'X';
                        for(k=0;k<2;k++){
                            set_uart_ID(k);
                            printf("ミル!! %c%d にあるプレイヤー %c の駒を消しました!!\n",'a'+j,i+1,not_player);
                        }
                        break;
                    }
                }
                if(number_ntp == 0)
                    break;
            }
        }
    }
    #endif
    return -1;
}


//Hit and blow

int Hit_and_Blow(){
    int flag,i,j;
    int question_size = 4;                                  // 問題の数列の要素数
    int max_turn = 8*2;                                     // 各プレイヤーの最大ターン数
    int hit,brow;                                           // hitとblowの数
    int turn = 0;                                           // 現在のターン数
    int seikai[question_size];                              // 正解の数列
    int data[max_turn][question_size+2];                    // 各ターンに提出された数列とその結果を保存
    char ch;
    char str[question_size > 4 ? question_size + 1 : 5];    // 入力された文字列を保存
    int uart_flag = 0;                                      // 0 → TeraTerm0(player1), 1 → TeraTerm1(player2)


    #if defined(NATIVE_MODE)
    printf("do nothing...\n");
    #else
    gpio->data = 0;


    // 乱数を用いて正解の数列を生成
    srand((unsigned int) time(NULL));
    for(i=0;i<question_size;i++){
        flag = 1;
        while(flag){
            seikai[i] = rand() % 10;
            flag = 0;
            for(j=0;j<i;j++)
                if(seikai[i] == seikai[j])
                    flag = 1;
        }
    }


    // 検証用
    for(i=0;i<question_size;i++){
        printf("%d ",seikai[i]);
    }
    printf("\n");




    while(1){
        turn++;
        // max_turn経過したとき
        if(turn > max_turn){
            for(i=0;i<2;i++){
                set_uart_ID(i);
                printf("ターンを超過しました。正解は");
                for(j=0;j<question_size;j++)
                    printf(" %d",seikai[j]);
                printf(" です。");
            }
            i = 0;
            break;
        }


        // ターンを持っていないプレイヤーに待つように要求
        set_uart_ID(!(turn % 2));
        printf(">>> Player %d の%dターン目です。\n--- 入力が処理されました。相手の入力を待っています... ---\n",turn % 2 + 1,(turn - 1) / 2 + 1);


        // 文字列の受け取り、文字列が数字のみになるまで繰り返す。
        flag = 1;
        set_uart_ID(turn % 2);
        while(flag){
            printf(">>> Player %d の%dターン目です。\n数字を %d つ入力してください : ",turn % 2 + 1,(turn - 1) / 2 + 1,question_size);
            fflush(stdout);
            for(i=0;i<question_size;i++)
                str[i] = io_getch();
            ch = io_getch();
            if(ch != '\n') printf("\n");
            flag = 0;
            for(i=0;i<question_size;i++){ // 文字列が数字のみであるなら受け取りを終了
                if(str[i] < '0' || '9' < str[i])
                    flag = 1;
            }
            // dataと入力していたなら、これまでの入力とその結果を表示し、入力待機状態に戻る
            if(str[0]=='d' && str[1]=='a' && str[2]=='t' && str[3]=='a'){
                for(j=0;j<turn-1;j++){
                    printf("    ");
                    for(i=0;i<question_size;i++) printf("%d ",data[j][i]);
                    printf("... %d hit %d brow\n",data[j][question_size],data[j][question_size+1]);
                }
            }
        }


        // 入力文字列の保存、hitとblowの生成
        hit = 0;
        brow = 0;
        N1 = 0;
        for(i=0;i<question_size;i++){
            data[turn-1][i] = str[i] - '0';
            N1 = (N1 << 8) | int_to_gpio[data[turn-1][i]];
            for(j=0;j<question_size;j++){
                if(data[turn-1][i] == seikai[j]){
                    if(i == j)  hit++;
                    else        brow++;
                }
            }
        }
        for(i=0;i<2;i++){
            set_uart_ID(i);
            printf("    ");
            for(j=0;j<question_size;j++)
                printf("%d ",data[turn-1][j]);
            printf("... %d hit %d brow\n",hit,brow);
        }


        data[turn-1][question_size] = hit;
        data[turn-1][question_size+1] = brow;
        // 7SEGに結果を表示
        N0 = (int_to_gpio[data[turn-1][4]] << 24) | (int_to_gpio[data[turn-1][5]] << 8) | 0b00000000011101000000000001111100;
        gpio->dout7SEG[1] = N1;
        gpio->dout7SEG[0] = N0;


        // 4ヒットのとき終了
        if(hit == 4){
            i = turn % 2 ? 2 : 1;
            break;
        }
    }
    #endif
    return i;
}


//Hit and Blow2
void Hit_and_Blow_2_print(unsigned short data[4]){
    for(int i=15;i>=0;i--){
        if(data[0] & 1 << i) printf("1");
        else            printf("0");
    }
    printf(" ... %d hit %d and %d or\n",(int)data[1],(int)data[2],(int)data[3]);
}




#if !defined(NATIVE_MODE)
void Hit_and_Blow_2_timer_handler(){
    if(counter == 0){
        gpio->dout7SEG[1] = (HiB_hit >= 10 ? int_to_gpio[1] << 8 : 0) | int_to_gpio[HiB_hit % 10];
        gpio->dout7SEG[0] = 0b00000000011101000000010001110000; // " hit"
    }
    else if(counter == 1){
        gpio->dout7SEG[1] = (HiB_and >= 10 ? int_to_gpio[1] << 8 : 0) | int_to_gpio[HiB_and % 10];
        gpio->dout7SEG[0] = 0b00000000011101110101010001011110; // " and"
    }
    else{
        gpio->dout7SEG[1] = (HiB_or  >= 10 ? int_to_gpio[1] << 8 : 0) | int_to_gpio[HiB_or  % 10];
        gpio->dout7SEG[0] = 0b00000000010111000101000000000000; // " or "
    }
    counter = (counter + 1) % 3;
}
#endif




int Hit_and_Blow_2(){
    printf("hello\n");
    unsigned short data[30][4],answer,sw;
    int i,j,turn,or,and,hit;
    char str[16],ch;
    #if defined(NATIVE_MODE)
    printf("do nothing...\n");
    #else
    set_uart_ID(0);
    printf("FPGAボードのスイッチをセットしてください。\n準備が完了したらいずれかのキーを押してください。\n");
    io_getch();
    answer = gpio->data;


    turn = 0;
    while(1){
        turn++;
        if(turn > 30){
            printf("ターン数を超過したので終了します。\n");
            for(i=15;i>=0;i--){
                if(answer & 1 << i) printf("1");
                else                printf("0");
                printf("です\n");
            }
            break;
        }
        printf("%d ターン目です。ビット列を入力してください。 >> ",turn);
        sw = 0;
        or = and = hit = 0;
        for(i=15;i>=0;i--){
            ch = io_getch();
            if(ch == 'd'){
                if(io_getch() == 'a' && io_getch() == 't' && io_getch() == 'a' && io_getch() == '\n'){
                    for(j=0;j<turn-1;j++){
                        printf("    ");
                        Hit_and_Blow_2_print(data[j]);
                    }
                }
            }
            if(ch != '0' && ch != '1'){
                i = 16;
                sw = 0;
                or = and = hit = 0;
                printf("\nビット列を入力してください。 >> ");
                fflush(stdout);
                continue;
            }
            sw = sw << 1 | (ch - '0');
            if(!((ch ^ answer >> i) & 1)) hit++;
            if(  (ch & answer >> i) & 1 ) and++;
            if(  (ch | answer >> i) & 1 ) or++;
        }
        data[turn-1][0] =           sw;
        data[turn-1][1] = HiB_hit = hit;
        data[turn-1][2] = HiB_and = and;
        data[turn-1][3] = HiB_or  = or ;
        printf("\n%d %d %d %d\n",sw,hit,and,or);
        Hit_and_Blow_2_print(data[turn-1]);
        set_mtimer_interval(1000/*msec*/);
        timer_interrupt_hook = Hit_and_Blow_2_timer_handler;
        enable_timer_interrupt(); //割り込みハンドラを設定するための関数
        if(hit == 16){
            printf("clear.");
            asm volatile("csrc mie, %[new]" : : [new] "r" (0x00000080));    /// clear meie[7] : machine-external-interrupt-enable
            break;
        }
    }
    #endif
    return turn;
}
