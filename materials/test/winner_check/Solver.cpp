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
        // 終局 (合法手なしで負け)
        // このノードの勝者は「相手」
        node_ptr->game_value = "UNKNOWN";
        node_ptr->outcome_class = "Loss"; // (CGTなら "P" - Previous player wins)
        node_ptr->is_optimal = false;
        node_ptr->winner = -(game.player); // 相手の勝ち
        return node_ptr;
    }

    std::vector<GameNode*> child_nodes;
    // bool immediate_win = false; // ループ後の判定で統一するため不要

    for (int m : moves) {
        auto [next_game, captured] = game.make_move(m);
        std::string child_key = make_key(next_game.board, next_game.player);
        node_ptr->children[child_key] = std::to_string(m); // 子ノードへの参照を記録
                                                                                                                                                                                                                     
        GameNode* child_node = nullptr;

        if (captured) {
            // 捕獲＝この手で即勝利。
            // 子ノード（相手の手番）は「負け」の終局ノードとなる。
            
            // 子ノードがまだ map になければ、"Loss" (負け) として作成・登録
            if (!nodes.count(child_key)) {
                auto child_node_obj = std::make_unique<GameNode>(next_game.board, next_game.player);
                child_node_obj->outcome_class = "UNKNOWN"; // 相手は負け
                child_node_obj->winner = game.player;  // 勝者は自分 (game.player)
                child_node_obj->game_value = "Lost (Captured)";
                child_node_obj->is_optimal = false; // 終局扱い
                nodes[child_key] = std::move(child_node_obj);
            }
            child_node = nodes[child_key].get();
            // この場合、子ノードは終局なので _find_value(next_game) は呼ばない
        } else {
            // 捕獲ではない場合、通常通り再帰的に探索
            child_node = _find_value(next_game);
        }
        
        // このループで調べた子ノードをリストに追加
        child_nodes.push_back(child_node);
    }




    // 勝敗判定 (Minimax法) すべての子ノードの結果を見て現在のノードの勝敗を決定する
    // winnerをみることで探索している


    bool found_win = false; // 自分が勝てる手（＝相手が負ける手）を見つけたか
    
    for (GameNode* child : child_nodes) {
        // 子ノード (相手の手番) の勝者 (child->winner) が、
        // 自分 (game.player) であれば、それは相手の負けを意味する。
        if (child->winner == game.player) { 
            found_win = true;
            // 全探索するため break しない
        }
    }

    if (found_win) {
        // 一つでも勝てる手があれば、このノード（自分）は「勝ち」
        node_ptr->winner = game.player;
        node_ptr->outcome_class = "Win"; // (CGTなら "N" - Next player wins)
        node_ptr->game_value = "UNKNOWN";
    } else {
        // 全ての手が（相手から見て）勝ちにつながる
        // ＝ どの手を選んでも自分は「負け」
        node_ptr->winner = -game.player;

    }

    //  細かいところは"UNKNOWN"にする 
    node_ptr->game_value = "UNKNOWN";
    node_ptr->outcome_class = "UNKNOWN";
     node_ptr->is_optimal = false;
    
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

void Solver::print_minimax_summary() const {
    std::cout << "---  Minimax Summary ---" << "\n";
    std::cout << "(Key: [Player] -> Winner: W, Optimal: O, Children: [...])\n\n";

    for (const auto& [key, node] : nodes) {
        // キーと手番
        std::cout << key << ": "
                  << "[" << (node->player_to_move == 1 ? "Black" : "White") << "] -> ";
        
        // Minimaxの結果 (Winner と Optimal)
        std::cout << "W: " << node->winner
                  << ", O: " << (node->is_optimal ? "Yes" : "No") << "\n";

        // 子ノード (どの手に遷移するか)
        std::cout << "    Children: ";
        if (node->children.empty()) {
            std::cout << "(Terminal)";
        } else {
            for (const auto& [child_id, move_str] : node->children) {
                // 子ノードのキーだけ表示
                std::cout << child_id << " "; 
            }
        }
        std::cout << "\n----------------\n";
    }
}
//void Solver::print_optinal_sort() const {}