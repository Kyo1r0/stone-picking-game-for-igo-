#include "Solver.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

int main() {
    int first, max_n;

    std::cout << "from:";
    scanf("%d", &first);

    std::cout << "to:";
    scanf("%d", &max_n);

    std::string filename =
        "analysis" + std::to_string(first) + "-" + std::to_string(max_n) + ".csv";

    std::cout << "Analysis 1x" << first << " to 1x" << max_n << "\n";
    std::cout << "Format: N, Result_Map (g=Win, r=Lose, x=Illegal)\n";
    std::cout << "------------------------------------------------\n";

    std::ofstream file(filename);
    file << "N,Map\n";

    for (int n = first; n <= max_n; ++n) {
        auto start = std::chrono::high_resolution_clock::now();

        Solver solver;
        std::string map_str = solver.analyze_initial_moves(n);

        auto end = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration<double>(end - start).count();

        std::cout << "N=" << n << " : [" << map_str << "] (" << sec << "s)\n";
        file << n << "," << map_str << "\n";
    }

    std::cout << "Done. Check " << filename << "\n";
    return 0;
}
