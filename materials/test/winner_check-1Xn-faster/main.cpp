#include "Solver.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

int main() {
    int from, to;

    std::cout << "from:";
    std::cin >> from;
    std::cout << "to:";
    std::cin >> to;

    std::string filename =
        "analysis" + std::to_string(from) + "-" + std::to_string(to) + ".csv";

    std::cout << "Analysis 1x" << from << " to 1x" << to << "\n";
    std::cout << "Format: N, Result_Map (g=Win, r=Lose, x=Illegal)\n";
    std::cout << "------------------------------------------------\n";

    std::ofstream ofs(filename);
    ofs << "N,Map\n";

    for (int n = from; n <= to; ++n) {
        auto start = std::chrono::high_resolution_clock::now();

        Solver solver;
        std::string result = solver.analyze_initial_moves(n);

        auto end = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration<double>(end - start).count();

        std::cout << "N=" << n << " : [";
        for (int i = 0; i < (int)result.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << result[i];
        }
        std::cout << "] (" << sec << "s)\n";

        ofs << n << "," << result << "\n";
    }

    std::cout << "Done. Check " << filename << "\n";
    return 0;
}
