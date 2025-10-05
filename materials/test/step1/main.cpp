#include <iostream>
#include "MiniGo1x3.h"
#include "Solver.h"

int main() {
    MiniGo1x3 board({0,0,0}, 1);

    std::cout << "Legal moves: ";
    for (int m : board.get_legal_moves()) std::cout << m << " ";
    std::cout << "\n";

    auto [new_board, captured] = board.make_move(0); 
    std::cout << "After move 0, captured? " << (captured ? "Yes" : "No") << "\n";
    std::cout << "New board: ";
    for (int x : new_board.board) std::cout << x << " ";
    std::cout << "\n";

    Solver solver;
    GameNode* root = solver.solve(board, 1);
    std::cout << "Number of nodes explored: " << solver.memo.size() << "\n";

    return 0;
}
