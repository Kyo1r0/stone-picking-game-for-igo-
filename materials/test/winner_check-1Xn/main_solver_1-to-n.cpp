#include "Solver.h"
#include <iostream>
#include <vector>
#include <chrono>

int main() {
    int max_n;
    std::cout << "Enter max board size N (e.g., 5): ";
    if (!(std::cin >> max_n)) return 0;

    std::cout << "\nAnalysis from 1x3 to 1x" << max_n << ":\n";
    std::cout << "----------------------------------------\n";



    for (int n = 3; n <= max_n; ++n) {
        Solver solver;
        
        // サイズ N の空の盤面を作成 (0で初期化)
        std::vector<int> initial_board(n, 0);
        int first_player = 1; // 黒番


        auto start = std::chrono::high_resolution_clock::now();
        // 解析実行
        solver.solve(initial_board, first_player);

        auto end = std::chrono::high_resolution_clock::now();

        // ★経過時間（秒）
        double elapsed_sec = std::chrono::duration<double>(end - start).count();

        // 勝者取得
        int winner = solver.get_initial_winner(initial_board, first_player);

        // 出力
        std::cout << "Size 1x" << n << " : ";
        if (winner == 1) {
            std::cout << "Black (First Player) Wins";
        } else if (winner == -1) {
            std::cout << "White (Second Player) Wins";
        } else {
            std::cout << "Draw / Unknown";
        }

        std::cout << " (Nodes: " << solver.num_nodes()
                  << ", Time: " << elapsed_sec << " s)\n";
    }

    std::cout << "----------------------------------------\n";

    return 0;
}