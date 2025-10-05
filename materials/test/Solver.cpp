#include "Solver.h"
#include <iostream>

GameNode* Solver::_find_value(const MiniGo1x3& board, int player) {
    std::string key = board.make_key(player);
    if (memo.find(key) != memo.end()) return memo[key].get();

    auto node = std::make_unique<GameNode>();
    node->id = key;
    node->board_state = board.board;
    node->player_to_move = player;
    node->game_value = "UNKNOWN";
    node->outcome_class = "UNKNOWN";
    node->is_optimal = false;

    auto legal_moves = board.get_legal_moves();
    for (int move : legal_moves) {
        auto [new_board, captured] = board.make_move(move);
        std::string child_key = new_board.make_key(-player);
        GameNode* child_node = _find_value(new_board, -player);
        node->children[std::to_string(move)] = child_key;
    }

    memo[key] = std::move(node);
    return memo[key].get();
}

GameNode* Solver::solve(const MiniGo1x3& initial_board, int initial_player) {
    return _find_value(initial_board, initial_player);
}
