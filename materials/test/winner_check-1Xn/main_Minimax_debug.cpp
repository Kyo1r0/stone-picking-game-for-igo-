#include "Solver.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream> // ファイル操作に必要

int main() {
    int n;
    // ユーザー入力（ここはコンソールに出す）
    std::cout << "Enter board size N (e.g., 3, 4, 5...): ";
    if (!(std::cin >> n)) return 0;

    // 1. 出力用ファイル名の作成 (例: Minimax1x3.txt)
    std::string filename = "Minimax1x" + std::to_string(n) + ".txt";
    std::ofstream outfile(filename);

    if (!outfile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    std::cout << "Running solver for 1x" << n << " ...\n";
    std::cout << "Result will be saved to [" << filename << "]\n";

    // 2. 標準出力(cout)のバッファをファイルに切り替える
    // これ以降、std::cout への出力はすべて画面ではなくファイルに書き込まれます
    std::streambuf* original_cout_buffer = std::cout.rdbuf(); 
    std::cout.rdbuf(outfile.rdbuf());

    // --- ここからファイルへの書き込み開始 ---

    Solver solver;
    std::vector<int> initial_board(n, 0);
    int first_player = 1; // 1 = Black

    // 解析実行
    solver.solve(initial_board, first_player);

    // 勝者取得
    int winner = solver.get_initial_winner(initial_board, first_player);

    std::cout << "========================================" << "\n";
    std::cout << " Board Size: 1x" << n << "\n";
    std::cout << " Initial Winner: " << winner;
    
    if (winner == 1) std::cout << " (Black Wins)";
    else if (winner == -1) std::cout << " (White Wins)";
    else std::cout << " (Draw/Unknown)";
    
    std::cout << "\n========================================" << "\n\n";
    
    // Minimaxの結果出力 (Solver内部のcoutもファイルに書き込まれます)
    if (n <= 100) {
        std::cout << "--- Minimax Summary ---\n";
        solver.print_minimax_summary();
    } else {
        std::cout << "Skipping detailed Minimax summary for N > 100.\n";
    }

    std::cout << "\nTotal nodes explored: " << solver.num_nodes() << "\n";

    // --- ファイルへの書き込み終了 ---

    // 3. 標準出力を元のコンソールに戻す
    std::cout.rdbuf(original_cout_buffer);

    // 完了メッセージ（コンソールに表示）
    std::cout << "Done! Please check " << filename << std::endl;

    return 0;
}