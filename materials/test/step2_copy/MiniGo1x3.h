#pragma once
#include <vector>
#include <utility>

class MiniGo1x3 {
public:
    std::vector<int> board;         // 0=空, 1=黒, -1=白
    int player;                     // 1=黒, -1=白
    
    // --- ▼▼▼ 変更点 1: 一手前の盤面を保存するメンバ変数を追加 ▼▼▼ ---
    std::vector<int> previous_board; // コウ判定用

    // --- ▼▼▼ 変更点 2: 新しいコンストラクタを2つ用意 ▼▼▼ ---
    // 初期盤面用のコンストラクタ
    MiniGo1x3(const std::vector<int>& board, int player);
    // 探索中に使う、一手前の盤面も受け取るコンストラクタ
    MiniGo1x3(const std::vector<int>& board, int player, const std::vector<int>& prev_board);

    std::vector<int> get_legal_moves() const;
    std::pair<MiniGo1x3, bool> make_move(int move) const;

private:
    bool is_capture(int pos, int current_player) const;
    bool would_be_suicide(int pos) const;
    int count_liberties(int pos, int color, const std::vector<int>& b) const;
};