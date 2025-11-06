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
        node_ptr->game_value = "0";  // 合法手なしで負け
        node_ptr->outcome_class = "P";
        node_ptr->is_optimal = false;
        node_ptr->winner = -(game.player);
        return node_ptr;
    }


    
    std::vector<GameNode*> child_nodes;
    bool immediate_win = false; // この手番で即勝利できるか


    for (int m : moves) {
        auto [next_game, captured] = game.make_move(m);
       

        if (captured) { //再帰の終端条件
            immediate_win = true; // 判定するために設けた
            
            // 1. このノード(自分)は "P" (勝ち) と確定
            node_ptr->is_optimal = true;
            node_ptr->winner = game.player;
            node_ptr->outcome_class = "P";
            node_ptr->game_value = "Win (Capture)";
            
            // 2. 子ノード(相手)を「手動で」設定する (再帰しない)
            std::string child_key = make_key(next_game.board, next_game.player);
            node_ptr->children[child_key] = std::to_string(m); //

            // 3. 子ノードがまだ map になければ、"L" (負け) として作成・登録
            if (!nodes.count(child_key)) {
                auto child_node_obj = std::make_unique<GameNode>(next_game.board, next_game.player);
                child_node_obj->outcome_class = "L"; // 相手は負け (L-position)
                child_node_obj->winner = game.player;  // 勝者は自分
                child_node_obj->game_value = "Lost (Terminal)";
                child_node_obj->is_optimal = true; // 終局扱い
                nodes[child_key] = std::move(child_node_obj);
            }
            // (child_nodes.push_back(nodes[child_key].get()); は break するため不要)

            // 4. 他の手の探索を打ち切る
            //break;
        }

        



        GameNode* child_node = _find_value(next_game);

        node_ptr->children[make_key(next_game.board, next_game.player)] = std::to_string(m);
        child_nodes.push_back(child_node);
    }

    // 勝敗・最善手判定（1×3用の簡単ルール）
    // 捕獲が起こる手があれば、自分勝利（"P"）
    bool found_win = false;
    for (GameNode* child : child_nodes) {
        if (child->outcome_class == "L") { // 相手が負ける手がある → 自分勝ち
            found_win = true;
            node_ptr->is_optimal = true; // 最善手
            break;
        }
    }
    
    node_ptr->game_value = "UNKNOWN";
    node_ptr->outcome_class = "UNKNOWN";
    node_ptr->is_optimal = false; // 最善手かどうかも不明なためfalseに設定
    
    return node_ptr;
}


void Solver::print_all_nodes() const {
    for (const auto& [key, node] : nodes) { //nodesがsolver.hで定義済み
        std::cout << "Node key: " << key << "\n  Board: ";
        for (int v : node->board_state) std::cout << v << " ";
        std::cout << "\n  Player: " << (node->player_to_move==1?"Black":"White")
                  << ", Value: " << node->game_value
                  << ", Outcome: " << node->outcome_class
                  << ", Optimal: " << (node->is_optimal?"Yes":"No") << "\n";
        std::cout << "  Children: ";
        for (const auto& [child_id, move_str] : node->children)
            std::cout << move_str << "(" << child_id << ") ";

        if (node->children.empty()) {
            std::cout << "\n  Winner: " << node->winner;
        }
        std::cout << "\n";
    }
}


int Solver::get_initial_winner() const {
    std::string root_key = make_key({0,0,0}, 1); // 初期設定に合わせて変更
    if (nodes.count(root_key)) {
        return nodes.at(root_key)->winner;
    }
    return 0; // 未探索時
}


//void Solver::print_optinal_sort() const {}