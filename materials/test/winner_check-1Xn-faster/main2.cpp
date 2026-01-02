#include "MiniGoBit.h"
#include <iostream>
#include <fstream>
#include <chrono>

int main() {
    int from, to;
    std::cout << "1xN MiniGo Solver (Bitboard Optimized)\n";
    std::cout << "Enter range N (e.g. 1 40)\n";
    std::cout << "From: ";
    std::cin >> from;
    std::cout << "To: ";
    std::cin >> to;

    std::string filename = "analysis_bit_" + std::to_string(from) + "-" + std::to_string(to) + ".csv";
    std::ofstream ofs(filename);
    ofs << "N,Result_Map\n";

    // ソルバーの初期化 (メモリ確保はここでのみ行われる)
    MiniGoBit solver;

    for (int n = from; n <= to; ++n) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::string res = solver.analyze(n);
        
        auto end = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration<double>(end - start).count();

        std::cout << "N=" << n << " : [" << res << "] (" << sec << "s)\n";
        ofs << n << "," << res << "\n";
    }

    std::cout << "Done. Saved to " << filename << "\n";
    return 0;
}