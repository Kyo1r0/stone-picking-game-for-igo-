#include "Solver.h"

int main() {
    std::vector<int> initial_board = {0,0,0};
    int initial_player = 1; // Black

    Solver solver;
    solver.solve(initial_board, initial_player);

    return 0;
}
