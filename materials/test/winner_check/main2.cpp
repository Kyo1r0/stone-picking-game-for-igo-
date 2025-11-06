#include "Solver.h"
#include <iostream>

int main() {
    Solver solver;
    solver.solve({0,0,0}, 1); // 初期盤面、黒番
    std::cout << "Number of nodes explored: " << solver.num_nodes() << "\n";
    
    solver.print_all_nodes();
    return 0;
}
