#pragma once
#include <vector>
#include <utility>

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
    
    // 修正: グループ（連）を考慮して呼吸点を数える
    int count_liberties(int pos, int color, const std::vector<int>& b) const;
    
    // 追加: 指定した位置にある石のグループ全体を削除する
    void remove_group(int pos, std::vector<int>& b) const;
};