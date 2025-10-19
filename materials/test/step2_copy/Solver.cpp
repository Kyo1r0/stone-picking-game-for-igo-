#include "Solver.h"
#include <iostream>
#include <sstream>

void Solver::solve(const std::vector<int>& initial_board, int initial_player) {
    MiniGo1x3 game(initial_board, initial_player);
    _find_value(game);
}

std::string Solver::make_key(const std::vector<int>& board, int player) const {
    std::ostringstream oss;
    oss << player << ":";
    for (int v : board) oss << v;
    return oss.str();
}

GameNode* Solver::_find_value(const MiniGo1x3& game) {
    std::string key = make_key(game.board, game.player);
    if (nodes.count(key)) return nodes[key].get();

    auto node = std::make_unique<GameNode>(game.board, game.player);
    GameNode* node_ptr = node.get();
    nodes[key] = std::move(node);

    auto moves = game.get_legal_moves();
    if (moves.empty()) {
        // 終局
        node_ptr->game_value = "UNKNOWN";  // 合法手なしで負け
        node_ptr->outcome_class = "P";
        node_ptr->is_optimal = false;
        return node_ptr;
    }


    //再帰呼び出し
    std::vector<GameNode*> child_nodes;
    for (int m : moves) {
        auto [next_game, captured] = game.make_move(m);
        GameNode* child_node = _find_value(next_game);

        node_ptr->children[make_key(next_game.board, next_game.player)] = std::to_string(m);
        child_nodes.push_back(child_node);
    }



    // 勝敗・最善手判定（1×3用の簡単ルール）
    // 捕獲が起こる手があれば、自分勝利（"P"）
    /* 後に実装
    bool found_win = false;
    for (GameNode* child : child_nodes) {
        if (child->outcome_class == "L") { // 相手が負ける手がある → 自分勝ち
            found_win = true;
            node_ptr->is_optimal = true; // 最善手
            break;
        }
    }
    node_ptr->game_value = found_win ? "P" : "N";
    node_ptr->outcome_class = found_win ? "P" : "N";
    */
 
    //とりあえず,UNKNOWNで対応
    node_ptr->game_value = "UNKNOWN";
    node_ptr->outcome_class = "UNKNOWN";
    node_ptr->is_optimal = false; // 最善手かどうかも不明なためfalseに設定


    return node_ptr;
}


void Solver::print_all_nodes() const {
    for (const auto& [key, node] : nodes) {
        std::cout << "Node key: " << key << "\n  Board: ";
        for (int v : node->board_state) std::cout << v << " ";
        std::cout << "\n  Player: " << (node->player_to_move==1?"Black":"White")
                  << ", Value: " << node->game_value
                  << ", Outcome: " << node->outcome_class
                  << ", Optimal: " << (node->is_optimal?"Yes":"No") << "\n";
        std::cout << "  Children: ";
        for (const auto& [child_id, move_str] : node->children)
            std::cout << move_str << "(" << child_id << ") ";
        std::cout << "\n";
        // std::flushをするといいかも
        //1*3で結論をなにか出すといい
    }
}
