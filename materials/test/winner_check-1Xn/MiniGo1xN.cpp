#include "MiniGo1xN.h"
#include <iostream>

MiniGo1xN::MiniGo1xN(const std::vector<int>& b, int p)
    : board(b), player(p) {}

// ★変更点: 境界チェックを board.size() に依存させる
int MiniGo1xN::count_liberties(int pos, int color, const std::vector<int>& b) const {
    int liberties = 0;
    int size = (int)b.size(); // サイズ取得
    if (pos > 0 && b[pos-1] == 0) liberties++;
    if (pos < size - 1 && b[pos+1] == 0) liberties++; // 2 -> size-1
    return liberties;
}

bool MiniGo1xN::is_capture(int pos, int current_player) const {
    bool captured = false;
    int opponent = -current_player;
    std::vector<int> tmp = board;
    int size = (int)board.size(); // サイズ取得

    tmp[pos] = current_player;
    for (int adj : {pos-1, pos+1}) {
        // ★変更点: adj <= 2 を adj < size に変更
        if (adj >= 0 && adj < size && tmp[adj] == opponent) {
            if (count_liberties(adj, opponent, tmp) == 0) {
                captured = true;
            }
        }
    }
    return captured;
}

bool MiniGo1xN::would_be_suicide(int pos) const {
    std::vector<int> tmp = board;
    tmp[pos] = player;
    if (is_capture(pos, player)) return false;
    return count_liberties(pos, player, tmp) == 0;
}

std::vector<int> MiniGo1xN::get_legal_moves() const {
    std::vector<int> moves;
    int size = (int)board.size(); // サイズ取得
    // ★変更点: i < 3 を i < size に変更
    for (int i = 0; i < size; i++) {
        if (board[i] == 0 && !would_be_suicide(i)) {
            moves.push_back(i);
        }
    }
    return moves;
}

std::pair<MiniGo1xN, bool> MiniGo1xN::make_move(int move) const {
    std::vector<int> new_board = board;
    new_board[move] = player;
    bool captured = false;
    int size = (int)board.size(); // サイズ取得

    int opponent = -player;
    for (int adj : {move-1, move+1}) {
        // ★変更点: adj <= 2 を adj < size に変更
        if (adj >= 0 && adj < size && new_board[adj] == opponent) {
            if (count_liberties(adj, opponent, new_board) == 0) {
                new_board[adj] = 0;
                captured = true;
            }
        }
    }

    MiniGo1xN next(new_board, -player);
    return {next, captured};
}