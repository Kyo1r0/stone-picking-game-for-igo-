#include "MiniGo1x3.h"
#include <sstream>

MiniGo1x3::MiniGo1x3(const std::vector<int>& b, int p)
    : board(b), player(p) {}

// 盤面＋手番から一意のキー文字列を作成
std::string MiniGo1x3::make_key(int player) const {
    std::ostringstream oss;
    oss << player << ":";
    for (int v : board) {
        oss << v;
    }
    return oss.str();
}

// 呼吸点の簡易判定
int MiniGo1x3::get_liberty(int index, int color) const {
    int liberty = 0;
    if (index > 0 && board[index-1] != color) liberty++;
    if (index < 2 && board[index+1] != color) liberty++;
    return liberty;
}

// 自殺手を考慮して合法手を返す
std::vector<int> MiniGo1x3::get_legal_moves() const {
    std::vector<int> moves;
    for (int i = 0; i < 3; ++i) {
        if (board[i] != 0) continue; // 空でない
        // 仮置きして呼吸点確認
        std::vector<int> tmp_board = board;
        tmp_board[i] = player;
        bool suicide = true;
        if (get_liberty(i, player) > 0) suicide = false;
        if (suicide) continue;
        moves.push_back(i);
    }
    return moves;
}

// 石を置く処理（簡易版）
std::pair<MiniGo1x3, bool> MiniGo1x3::make_move(int move) const {
    std::vector<int> new_board = board;
    new_board[move] = player;
    bool captured = false;
    // 左右の相手石をチェック
    for (int dir : {-1,1}) {
        int idx = move + dir;
        if (idx < 0 || idx > 2) continue;
        if (new_board[idx] == -player && get_liberty(idx, -player) == 0) {
            new_board[idx] = 0;
            captured = true;
        }
    }
    MiniGo1x3 next(new_board, -player);
    return {next, captured};
}
