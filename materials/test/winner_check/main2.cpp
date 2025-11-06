#include "Solver.h"
#include <iostream>

int main() {
    Solver solver;
    
   
    solver.solve({0,0,0}, 1); // 初期盤面、黒番


    int winner = solver.get_initial_winner();
    
    std::cout << "========================================" << "\n";
    std::cout << " Winner Result " << "\n";
    if (winner == 1) {
        std::cout << "Black (First Player) Wins!" << "\n";
    } else if (winner == -1) {
        std::cout << "White (Second Player) Wins!" << "\n";
    } else {
        std::cout << "It's a Draw (or undecided)." << "\n";
    }


    std::cout << "========================================" << "\n\n";



    std::cout << "\nTotal number of nodes explored: " << solver.num_nodes() << "\n";
    std::cout << "--- All Explored Nodes  ---" << "\n";
    solver.print_all_nodes();
    
    
    return 0;
}