#pragma once
#include <vector>
#include <utility>

class MiniGo1x3 {
public:
    std::vector<int> board; // 0=空, 1=黒, -1=白
    int player;             // 1=黒, -1=白

    MiniGo1x3(const std::vector<int>& board, int player);

    std::vector<int> get_legal_moves() const;
    std::pair<MiniGo1x3,bool> make_move(int move) const;

private:
    bool is_capture(int pos, int current_player) const;
    bool would_be_suicide(int pos) const;
    int count_liberties(int pos, int color, const std::vector<int>& b) const;
};
