#include "Solver.h"
#include <iostream>

void Solver::solve(const std::vector<int>& initial_board, int initial_player) {
    MiniGo1x3 board(initial_board, initial_player);
    _find_value(board, initial_player);
    std::cout << "Number of nodes explored: " << memo.size() << std::endl;
    for (auto& [key, node_ptr] : memo) {
        std::cout << "Node key: " << key << std::endl;
        std::cout << "  Board: ";
        for (int v : node_ptr->board_state) std::cout << v << " ";
        std::cout << "\n  Player: " << (node_ptr->player_to_move == 1 ? "Black" : "White")
                  << ", Value: " << node_ptr->game_value
                  << ", Outcome: " << node_ptr->outcome_class
                  << ", Optimal: " << (node_ptr->is_optimal ? "Yes" : "No") << std::endl;
        std::cout << "  Children: ";
        for (auto& [move, child_key] : node_ptr->children)
            std::cout << move << "(" << child_key << ") ";
        std::cout << std::endl;
    }
}

GameNode* Solver::_find_value(const MiniGo1x3& board, int player) {
    std::string key = board.make_key(player);
    if (memo.find(key) != memo.end()) return memo[key].get();

    auto node_ptr = std::make_unique<GameNode>();
    node_ptr->id = key;
    node_ptr->board_state = board.board;
    node_ptr->player_to_move = player;
    node_ptr->game_value = "UNKNOWN";
    node_ptr->outcome_class = "UNKNOWN";
    node_ptr->is_optimal = false;

    // 子ノード展開
    std::vector<int> legal_moves = board.get_legal_moves();
    for (int move : legal_moves) {
        auto [new_board, captured] = board.make_move(move);
        std::string child_key = new_board.make_key(-player);
        node_ptr->children[std::to_string(move)] = child_key;
        if (memo.find(child_key) == memo.end()) {
            _find_value(new_board, -player);
        }
    }

    memo[key] = std::move(node_ptr);
    return memo[key].get();
}
