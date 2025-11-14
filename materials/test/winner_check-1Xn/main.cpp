#include "Solver.h"
#include <iostream>

int main() {
    int n;
    std::cin >> n;
    int first_player = 1; // 黒番
    Solver solver;
    std::vector<int> initial_board(n, 0); // 初期盤面、黒番
    std::cout << "Number of nodes explored: " << solver.num_nodes() << "\n";
    
    solver.print_all_nodes();
    return 0;
}
