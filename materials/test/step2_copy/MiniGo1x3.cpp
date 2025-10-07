#include "MiniGo1x3.h"
#include <iostream>

MiniGo1x3::MiniGo1x3(const std::vector<int>& b, int p)
    : board(b), player(p) {}

// 指定マスの呼吸点を数える
int MiniGo1x3::count_liberties(int pos, int color, const std::vector<int>& b) const {
    int liberties = 0;
    if (pos > 0 && b[pos-1] == 0) liberties++;
    if (pos < 2 && b[pos+1] == 0) liberties++;
    return liberties;
}

// 捕獲判定（隣の相手石の呼吸点が0になるか）
bool MiniGo1x3::is_capture(int pos, int current_player) const {
    bool captured = false;
    int opponent = -current_player;
    std::vector<int> tmp = board;

    // 仮置きして呼吸点0の相手石を捕獲
    tmp[pos] = current_player;
    for (int adj : {pos-1, pos+1}) {
        if (adj >= 0 && adj <= 2 && tmp[adj] == opponent) {
            if (count_liberties(adj, opponent, tmp) == 0) {
                captured = true;
            }
        }
    }
    return captured;
}

// 自分の石が置かれた結果呼吸点0になるか
bool MiniGo1x3::would_be_suicide(int pos) const {
    std::vector<int> tmp = board;
    tmp[pos] = player;
    // 捕獲があれば合法
    if (is_capture(pos, player)) return false;
    return count_liberties(pos, player, tmp) == 0;
}

// 合法手を返す
std::vector<int> MiniGo1x3::get_legal_moves() const {
    std::vector<int> moves;
    for (int i=0;i<3;i++) {
        if (board[i]==0 && !would_be_suicide(i)) {
            moves.push_back(i);
        }
    }
    return moves;
}

// 着手と捕獲処理
std::pair<MiniGo1x3,bool> MiniGo1x3::make_move(int move) const {
    std::vector<int> new_board = board;
    new_board[move] = player;
    bool captured = false;

    int opponent = -player;
    for (int adj : {move-1, move+1}) {
        if (adj >=0 && adj <=2 && new_board[adj]==opponent) {
            if (count_liberties(adj, opponent, new_board) == 0) {
                new_board[adj] = 0;
                captured = true;
            }
        }
    }

    MiniGo1x3 next(new_board, -player);
    return {next, captured};
}
