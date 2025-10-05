#include "MiniGo1x3.h"
#include <sstream>

MiniGo1x3::MiniGo1x3(const std::vector<int>& board_, int player_) :
    board(board_), player_to_move(player_) {}

bool MiniGo1x3::will_capture(int pos) const {
    int opponent = -player_to_move;
    // 左隣
    if (pos > 0 && board[pos - 1] == opponent) {
        bool left_liberty = false;
        if (pos - 2 < 0 || board[pos - 2] == 0) left_liberty = true;
        if (!left_liberty) return true;
    }
    // 右隣
    if (pos < 2 && board[pos + 1] == opponent) {
        bool right_liberty = false;
        if (pos + 2 > 2 || board[pos + 2] == 0) right_liberty = true;
        if (!right_liberty) return true;
    }
    return false;
}

std::vector<int> MiniGo1x3::get_legal_moves() const {
    std::vector<int> legal;
    for (int i = 0; i < 3; ++i) {
        if (board[i] != 0) continue;
        MiniGo1x3 temp(*this);
        auto [new_board, captured] = temp.make_move(i);
        bool suicide = true;
        for (int j = 0; j < 3; ++j) {
            if (new_board.board[j] == player_to_move) suicide = false;
        }
        if (!suicide || captured) legal.push_back(i);
    }
    return legal;
}

std::pair<MiniGo1x3, bool> MiniGo1x3::make_move(int move) const {
    MiniGo1x3 new_board(*this);
    new_board.board[move] = player_to_move;

    bool captured = false;
    int opponent = -player_to_move;

    // 左の石を取る
    if (move > 0 && new_board.board[move - 1] == opponent) {
        bool left_liberty = false;
        if (move - 2 < 0 || new_board.board[move - 2] == 0) left_liberty = true;
        if (!left_liberty) {
            new_board.board[move - 1] = 0;
            captured = true;
        }
    }

    // 右の石を取る
    if (move < 2 && new_board.board[move + 1] == opponent) {
        bool right_liberty = false;
        if (move + 2 > 2 || new_board.board[move + 2] == 0) right_liberty = true;
        if (!right_liberty) {
            new_board.board[move + 1] = 0;
            captured = true;
        }
    }

    new_board.player_to_move = opponent;
    return {new_board, captured};
}

std::string MiniGo1x3::make_key(int player) const {
    std::ostringstream oss;
    oss << player << ":";
    for (int x : board) oss << x;
    return oss.str();
}
