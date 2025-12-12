#include "MiniGo1xN.h"
#include <iostream>

MiniGo1xN::MiniGo1xN(const std::vector<int>& b, int p)
    : board(b), player(p) {}

// ★修正: グループ全体を見て呼吸点を数える
int MiniGo1xN::count_liberties(int pos, int color, const std::vector<int>& b) const {
    // そのマスが空点なら0を返す（または呼び出し側で制御）
    if (b[pos] != color) return 0;

    int size = (int)b.size();
    
    // 左端を探す
    int left = pos;
    while (left > 0 && b[left - 1] == color) {
        left--;
    }

    // 右端を探す
    int right = pos;
    while (right < size - 1 && b[right + 1] == color) {
        right++;
    }

    // グループの両端の外側が空いているかチェック
    int liberties = 0;
    if (left > 0 && b[left - 1] == 0) liberties++;
    if (right < size - 1 && b[right + 1] == 0) liberties++;

    return liberties;
}

// ★追加: グループ全体を削除する
void MiniGo1xN::remove_group(int pos, std::vector<int>& b) const {
    int color = b[pos];
    if (color == 0) return;
    int size = (int)b.size();

    // 左端を探す
    int left = pos;
    while (left > 0 && b[left - 1] == color) {
        left--;
    }

    // 右端を探す
    int right = pos;
    while (right < size - 1 && b[right + 1] == color) {
        right++;
    }

    // 範囲内をすべて0(空)にする
    for (int i = left; i <= right; ++i) {
        b[i] = 0;
    }
}

bool MiniGo1xN::is_capture(int pos, int current_player) const {
    bool captured = false;
    int opponent = -current_player;
    std::vector<int> tmp = board;
    int size = (int)board.size();

    tmp[pos] = current_player;
    
    // 左右の隣接石を確認
    for (int adj : {pos-1, pos+1}) {
        if (adj >= 0 && adj < size && tmp[adj] == opponent) {
            // その敵グループの呼吸点が0なら捕獲発生
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

    // 捕獲が発生するなら自殺手ではない（石を取って生きる場合があるため）
    if (is_capture(pos, player)) return false;

    // 置いた石（を含むグループ）の呼吸点が0なら自殺手
    return count_liberties(pos, player, tmp) == 0;
}

std::vector<int> MiniGo1xN::get_legal_moves() const {
    std::vector<int> moves;
    int size = (int)board.size();
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
    int size = (int)board.size();

    int opponent = -player;
    for (int adj : {move-1, move+1}) {
        if (adj >= 0 && adj < size && new_board[adj] == opponent) {
            if (count_liberties(adj, opponent, new_board) == 0) {
                // ★修正: 隣の石だけでなく、そのグループ全体を削除する
                remove_group(adj, new_board);
                captured = true;
            }
        }
    }

    MiniGo1xN next(new_board, -player);
    return {next, captured};
}