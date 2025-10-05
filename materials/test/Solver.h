#pragma once
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "MiniGo1x3.h"

struct GameNode {
    std::string id;
    std::vector<int> board_state;
    int player_to_move;
    std::map<std::string, std::string> children;
    std::string game_value;   // 現時点では "UNKNOWN"
    std::string outcome_class; // 現時点では "UNKNOWN"
    bool is_optimal;          // 現時点では false

    GameNode() : player_to_move(0), game_value("UNKNOWN"), outcome_class("UNKNOWN"), is_optimal(false) {}
};

class Solver {
public:
    Solver() {}

    void solve(const std::vector<int>& initial_board, int initial_player);
    GameNode* _find_value(const MiniGo1x3& board, int player);

    // 解析後に JSON 出力したい場合の関数（未実装）
    void save_to_json(const std::string& filepath) {}

    std::map<std::string, std::unique_ptr<GameNode>> memo;
};
