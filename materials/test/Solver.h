#ifndef SOLVER_H
#define SOLVER_H

#include <map>
#include <memory>
#include <vector>
#include <string>
#include "MiniGo1x3.h"

struct GameNode {
    std::string id;
    std::vector<int> board_state;
    int player_to_move;
    std::map<std::string, std::string> children;
    std::string game_value;
    std::string outcome_class;
    bool is_optimal;
};

class Solver {
public:
    std::map<std::string, std::unique_ptr<GameNode>> memo;
    GameNode* solve(const MiniGo1x3& initial_board, int initial_player);

private:
    GameNode* _find_value(const MiniGo1x3& board, int player);
};

#endif
