#pragma once
#include <vector>
#include <utility>

// クラス名を MiniGo1xN に変更（推奨）
class MiniGo1xN {
public:
    std::vector<int> board; 
    int player;             

    MiniGo1xN(const std::vector<int>& board, int player);

    std::vector<int> get_legal_moves() const;
    std::pair<MiniGo1xN, bool> make_move(int move) const;

private:
    bool is_capture(int pos, int current_player) const;
    bool would_be_suicide(int pos) const;
    int count_liberties(int pos, int color, const std::vector<int>& b) const;
};