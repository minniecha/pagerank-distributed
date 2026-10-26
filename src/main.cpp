#include "graph_io.hpp"
#include "pagerank.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <graph-file> [scores-output]\n";
        return 1;
    }

    try {
        const auto graph = pr::load_edge_list(argv[1]);

        std::cout << "Vertices: " << graph.num_vertices() << '\n';
        std::cout << "Edges: " << graph.num_edges() << '\n';

        const pr::PageRankOptions options;

        // Measure the algorithm call; file loading is excluded.
        const auto start = std::chrono::steady_clock::now();
        const auto result = pr::compute_pagerank(graph, options);
        const auto finish = std::chrono::steady_clock::now();

        const double elapsed_ms =
            std::chrono::duration<double, std::milli>(
                finish - start).count();

        const double score_sum = std::accumulate(
            result.scores.begin(), result.scores.end(), 0.0);

        // Export every vertex's score with enough precision.
        if (argc == 3) {
            std::ofstream output(argv[2]);
            if (!output) {
                throw std::runtime_error("Cannot open score output file");
            }

            output << std::setprecision(
                std::numeric_limits<double>::max_digits10);

            for (pr::VertexId u = 0; u < graph.num_vertices(); ++u) {
                output << u << ' ' << result.scores[u] << '\n';
            }

            output.close();
            if (!output) {
                throw std::runtime_error("Failed to write score output file");
            }
        }

        std::cout << "Converged: "
                  << (result.converged ? "yes" : "no") << '\n';
        std::cout << "Iterations: " << result.iterations << '\n';

        std::cout << std::scientific << std::setprecision(6);
        std::cout << "Final L1 change: "
                  << result.final_l1_change << '\n';

        std::cout << std::fixed << std::setprecision(12);
        std::cout << "Score sum: " << score_sum << '\n';

        std::cout << std::setprecision(3);
        std::cout << "PageRank time (ms): " << elapsed_ms << '\n';

        std::cout << std::setprecision(12);

        if (graph.num_vertices() <= 20) {
            std::cout << "Scores by vertex:\n";

            for (pr::VertexId u = 0; u < graph.num_vertices(); ++u) {
                std::cout << u << ' ' << result.scores[u] << '\n';
            }
        } else {
            std::vector<pr::VertexId> order(graph.num_vertices());
            std::iota(order.begin(), order.end(), pr::VertexId{0});

            const auto top_count =
                std::min<std::size_t>(10, order.size());

            std::partial_sort(
                order.begin(),
                order.begin() + top_count,
                order.end(),
                [&result](pr::VertexId a, pr::VertexId b) {
                    if (result.scores[a] == result.scores[b]) {
                        return a < b;
                    }
                    return result.scores[a] > result.scores[b];
                });

            std::cout << "Top " << top_count << " vertices:\n";

            for (std::size_t i = 0; i < top_count; ++i) {
                const auto u = order[i];
                std::cout << u << ' ' << result.scores[u] << '\n';
            }
        }

        if (!result.converged) {
            std::cerr << "PageRank reached the iteration limit.\n";
            return 2;
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
