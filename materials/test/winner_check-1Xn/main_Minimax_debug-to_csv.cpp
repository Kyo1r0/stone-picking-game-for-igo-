#include "Solver.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    int n;
    std::cout << "Enter board size N (e.g., 3, 4, 5...): ";
    if (!(std::cin >> n)) return 0;

    // 初期盤面（全部空）
    std::vector<int> board(n, 0);
    int first_player = 1;  // 黒番

    Solver solver;

    std::cout << "\nRunning full solver for 1x" << n << " ...\n";
    solver.solve(board, first_player);

    // 勝者
    int winner = solver.get_initial_winner(board, first_player);
    std::cout << "Winner = " << (winner == 1 ? "Black" : "White") << "\n";

    // --- CSV 出力 ---
    std::string csvname = "heatmap_1x" + std::to_string(n) + ".csv";
    std::cout << "Exporting heatmap to: " << csvname << "\n";

    solver.export_heatmap_csv(board, first_player, csvname);

    std::cout << "Done!\n";
    return 0;
}
