#include "MiniGoMT.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

int main() {
    int from, to;
    std::cout << "1xN MiniGo Solver (Parallel + Bitboard)\n";
    std::cout << "CPU Cores: " << std::thread::hardware_concurrency() << "\n";
    std::cout << "From: "; std::cin >> from;
    std::cout << "To: "; std::cin >> to;

    // メモリ量に合わせてTTサイズビット数を調整 (27 = 2GB, 24 = 256MB)
    // お使いのPCメモリが16GB以上なら 27 か 28 を推奨
    MiniGoMT solver(28); 

    std::string filename = "analysis_mt_" + std::to_string(from) + "-" + std::to_string(to) + ".csv";
    std::ofstream ofs(filename);
    ofs << "N,Result\n";

    for (int n = from; n <= to; ++n) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // 並列解析実行
        std::string res = solver.analyze_parallel(n);
        
        auto end = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration<double>(end - start).count();

        std::cout << "N=" << n << " : [" << res << "] (" << sec << "s)\n";
        ofs << n << "," << res << "\n";
    }
    return 0;
}