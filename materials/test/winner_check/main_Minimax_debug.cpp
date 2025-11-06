#include "Solver.h"
#include <iostream>



int main() {
    Solver solver;
    solver.solve({0,0,0}, 1);

    int winner = solver.get_initial_winner();
    std::cout << "========================================" << "\n";
    std::cout << " Initial Winner: " << winner << "\n";
    std::cout << "========================================" << "\n\n";
    
    // 詳細なログが見たい場合
    // solver.print_all_nodes(); 
    
    // ★Minimaxの伝播だけ見たい場合
    solver.print_minimax_summary();

    std::cout << "\nTotal nodes: " << solver.num_nodes() << "\n";
    return 0;
}