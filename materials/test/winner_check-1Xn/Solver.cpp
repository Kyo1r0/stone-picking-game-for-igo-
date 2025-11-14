#include "Solver.h"
#include <iostream>
#include <sstream>

void Solver::solve(const std::vector<int>& initial_board, int initial_player) {
    MiniGo1xN game(initial_board, initial_player);
    _find_value(game);
}

std::string Solver::make_key(const std::vector<int>& board, int player) const {
    std::ostringstream oss;
    oss << player << ":";
    for (int v : board) oss << v;
    return oss.str();
}


// (ヘルパー1) ノードの作成
GameNode* Solver::_create_new_node(const std::string& key, const MiniGo1xN& game) {
    auto node = std::make_unique<GameNode>(game.board, game.player);
    GameNode* node_ptr = node.get();
    nodes[key] = std::move(node);
    return node_ptr;
}

// (ヘルパー2) 終局ノードの設定 
void Solver::_setup_terminal_node(GameNode* node, int winner, const std::string& reason) {
    node->winner = winner;
    node->game_value = reason;
    node->outcome_class = "UNKNOWN"; 
    node->is_optimal = false;       // 終局なので
}

// (ヘルパー3) 探索と評価
void Solver::_explore_children_and_evaluate(GameNode* node_ptr, const MiniGo1xN& game, const std::vector<int>& moves){
    
    std::vector<GameNode*> child_nodes;

    for (int m : moves) {
        auto [next_game, captured] = game.make_move(m);
        std::string child_key = make_key(next_game.board, next_game.player);
        node_ptr->children[child_key] = std::to_string(m); 

        GameNode* child_node = nullptr;

        if (captured) {
            // 捕獲＝子ノードは終局
            if (!nodes.count(child_key)) {
                // 子ノードを作成し、終局として設定
                child_node = _create_new_node(child_key, next_game);
                _setup_terminal_node(child_node, game.player, "Lost (Captured)");
            } else {
                child_node = nodes[child_key].get();
            }
        } else {
            // 捕獲でない＝再帰
            child_node = _find_value(next_game);
        }
        child_nodes.push_back(child_node);
    }

    // (ヘルパー4) 評価
    search_winner_Minimax(node_ptr, child_nodes, game.player);
}

// (ヘルパー4)勝敗判定 (Minimax法) すべての子ノードの結果を見て現在のノードの勝敗を決定する
void Solver::search_winner_Minimax(GameNode* node_ptr, const std::vector<GameNode*>& child_nodes, int current_player){ 
    bool found_win = false;
    for (GameNode* child : child_nodes) {
        if (child->winner == current_player) { 
            found_win = true;
        }
    }

    if (found_win) {
        node_ptr->winner = current_player;
        node_ptr->outcome_class = "Win"; 
        node_ptr->game_value = "UNKNOWN";
    } else {
        node_ptr->winner = -current_player;
    }

    // 細かいところは"UNKNOWN"にする 
    node_ptr->game_value = "UNKNOWN";
    node_ptr->outcome_class = "UNKNOWN";
    node_ptr->is_optimal = false;
}


//メインで指揮をとっているだけ
GameNode* Solver::_find_value(const MiniGo1xN& game) {
    // 1. メモ化チェック (これは _find_value の責務)
    std::string key = make_key(game.board, game.player);
    if (nodes.count(key)) {
        return nodes[key].get();
    }

    // 2. ノードの作成
    GameNode* node_ptr = _create_new_node(key, game);

    // 3. 終局かどうかの判定と「振り分け」
    auto moves = game.get_legal_moves();
    if (moves.empty()) {
        // 終局処理をヘルパー2に任せる
        _setup_terminal_node(node_ptr, -(game.player), "Loss (No Moves)");
    } else {
        // 探索処理をヘルパー3に任せる
        _explore_children_and_evaluate(node_ptr, game, moves);
    }
    
    // 4. 設定済みのノードを返す
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


// 引数に合わせて修正。固定の {0,0,0} ではなく、引数からキーを作る
int Solver::get_initial_winner(const std::vector<int>& board, int player) const {
    std::string root_key = make_key(board, player); 
    if (nodes.count(root_key)) {
        return nodes.at(root_key)->winner;
    }
    return 0; 
}


void Solver::print_minimax_summary() const {
    std::cout << "---  Minimax Summary ---" << "\n";
    std::cout << "(Key: [Player] -> Winner: W, Optimal: O, Children: [...])\n\n";

    for (const auto& [key, node] : nodes) {
        std::cout << key << ": "
                  << "[" << (node->player_to_move == 1 ? "Black" : "White") << "] -> ";

        std::cout << "W: " << node->winner
                  << ", O: " << (node->is_optimal ? "Yes" : "No") << "\n";

        std::cout << "    Children: ";
        if (node->children.empty()) {
            std::cout << "(Terminal)";
        } else {
            for (const auto& [child_id, move_str] : node->children) {
                
                std::cout << child_id << " "; 
            }
        }
        std::cout << "\n----------------\n";
    }
}
//void Solver::print_optinal_sort() const {}

