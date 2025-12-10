#include "Solver.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    int n;
    std::cout << "Enter board size N: ";
    if (!(std::cin >> n)) return 0;

    Solver solver;
    std::vector<int> initial_board(n, 0); // 初期盤面 0:空
    int first_player = 1; // 黒番

    std::cout << "Solving 1x" << n << " ...\n";
    
    // 1. 全探索を実行（これでnodesにデータが溜まる）
    solver.solve(initial_board, first_player);

    // 勝者の表示
    int winner = solver.get_initial_winner(initial_board, first_player);
    std::cout << "Initial Winner: " << (winner == 1 ? "Black" : "White") << "\n";

    // 2. 結果をCSVに出力 (ナビゲーターアプリ用)
    std::string csv_filename = "game_map_1x" + std::to_string(n) + ".csv";
    solver.export_all_nodes_csv(csv_filename);

    return 0;
}