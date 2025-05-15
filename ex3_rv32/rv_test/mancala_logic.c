#include <stdio.h>
#include <stdlib.h>

#define PITS 14

void init_board(int board[]) {
    for (int i = 1; i <= PITS; i++) {
        board[i] = (i == 7 || i == 14) ? 0 : 4;
    }
}

void print_board(int board[]) {
    printf("\n      [相手]\n    ");
    for (int i = 13; i >= 8; i--) {
        printf(" %2d ", board[i]);
    }
    printf("\n %2d                          %2d\n", board[14], board[7]);
    printf("    ");
    for (int i = 1; i <= 6; i++) {
        printf(" %2d ", board[i]);
    }
    printf("\n      [自分]\n\n");
}

int is_game_over(int board[]) {
    int sumA = 0, sumB = 0;
    for (int i = 1; i <= 6; i++) sumA += board[i];
    for (int i = 8; i <= 13; i++) sumB += board[i];
    return (sumA == 0 || sumB == 0);
}

int main(void) {
    int board[PITS+1];
    init_board(board);

    int player = 1; 

    while (!is_game_over(board)) {
        print_board(board);
        printf("プレイヤー %d の番です。\n", player);

        int choice1;
        int pit;
        while (1) {
            printf("ポケット番号を 1～6 で選んでください：");
            if (scanf("%d", &choice1) != 1) {
                fprintf(stderr, "入力エラー\n");
                return 1;
            }
            if (choice1 < 1 || choice1 > 6) {
                printf("1～6 の範囲で入力してください。\n");
                continue;
            }
            // 実際のインデックスにマッピング
            pit = (player == 1) ? choice1 : choice1 + 7;
            if (board[pit] == 0) {
                printf("そのポケットには石がありません。他を選んでください。\n");
                continue;
            }
            break;
        }

        int stones = board[pit];
        board[pit] = 0;
        int pos = pit;

        while (stones > 0) {
            pos = pos % PITS + 1;
            // 相手のゴールをスキップ
            if ((player == 1 && pos == 14) || (player == 2 && pos == 7))
                continue;
            board[pos]++;
            stones--;
        }

        // 最後の石が自ゴールならもう一度
        int own_goal = (player == 1 && pos == 7) || (player == 2 && pos == 14);
        if (!own_goal) {
            player = (player == 1) ? 2 : 1;
        }
    }

    // 終了後、残り石を各ゴールに回収
    for (int i = 1; i <= 6; i++) {
        board[7] += board[i];
        board[i] = 0;
    }
    for (int i = 8; i <= 13; i++) {
        board[14] += board[i];
        board[i] = 0;
    }
    print_board(board);

    int stonesA = board[7], stonesB = board[14];
    printf("最終結果：1ゴール=%d, 2ゴール=%d\n", stonesA, stonesB);
    if (stonesA > stonesB)
        printf("プレイヤー 1 の勝利！\n");
    else if (stonesB > stonesA)
        printf("プレイヤー 2 の勝利！\n");
    else
        printf("引き分けです。\n");

    return 0;
}
