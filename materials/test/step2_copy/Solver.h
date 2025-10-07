#pragma once
#include "MiniGo1x3.h"
#include <string>
#include <map>
#include <memory>
#include <vector>

struct GameNode {
    std::string id;                     // ノードID
    std::vector<int> board_state;       // 盤面
    int player_to_move;                 // 手番
    std::map<std::string, std::string> children; // child_id -> move_str
    std::string game_value;             // ゲーム値 ("P","N","L","R" など)
    std::string outcome_class;          // 勝敗分類
    bool is_optimal;                    // 最善手フラグ

    GameNode(const std::vector<int>& board, int player)
        : board_state(board), player_to_move(player), game_value(""), outcome_class(""), is_optimal(false) {}
};

class Solver {
public:
    Solver() {}

    // 初期盤面・初期手番から解析を開始
    void solve(const std::vector<int>& initial_board, int initial_player);

    // メモ化マップに格納されたノード数を取得
    size_t num_nodes() const { return nodes.size(); }

    // デバッグ用：全ノード出力e
    void print_all_nodes() const;

private:
    std::map<std::string, std::unique_ptr<GameNode>> nodes;

    // 再帰探索関数
    GameNode* _find_value(const MiniGo1x3& game);

    // 盤面と手番からノードキーを生成
    std::string make_key(const std::vector<int>& board, int player) const;
};
