#include "pagerank.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pr {

PageRankResult compute_pagerank(
    const CSRGraph& graph,
    const PageRankOptions& options
) {
    const auto n = graph.num_vertices();

    if (n == 0) {
        throw std::invalid_argument("PageRank requires at least one vertex");
    }

    if (!std::isfinite(options.damping) ||
        options.damping < 0.0 || options.damping >= 1.0) {
        throw std::invalid_argument("Damping must be in [0, 1)");
    }

    if (!std::isfinite(options.tolerance) ||
        options.tolerance <= 0.0 ||
        options.max_iterations == 0) {
        throw std::invalid_argument(
            "Tolerance and maximum iterations must be positive");
    }

    const double vertex_count = static_cast<double>(n);

    // Start with equal probability at every vertex.
    std::vector<double> rank(n, 1.0 / vertex_count);
    std::vector<double> next_rank(n, 0.0);

    PageRankResult result;

    for (std::size_t iteration = 0;
         iteration < options.max_iterations;
         ++iteration) {
        // Sum the scores of vertices without outgoing edges.
        double dangling_mass = 0.0;

        for (VertexId u = 0; u < n; ++u) {
            if (graph.out_degree(u) == 0) {
                dangling_mass += rank[u];
            }
        }

        // Every vertex receives random-jump probability and
        // an equal share of dangling mass.
        const double base =
            (1.0 - options.damping) / vertex_count +
            options.damping * dangling_mass / vertex_count;

        std::fill(next_rank.begin(), next_rank.end(), base);

        // Distribute each source vertex's score along outgoing edges.
        for (VertexId u = 0; u < n; ++u) {
            const auto degree = graph.out_degree(u);

            if (degree == 0) {
                continue;
            }

            const double contribution =
                options.damping * rank[u] /
                static_cast<double>(degree);

            for (EdgeOffset edge = graph.row_offsets[u];
                 edge < graph.row_offsets[u + 1];
                 ++edge) {
                const auto v = graph.col_indices[edge];
                next_rank[v] += contribution;
            }
        }

        // L1 change measures the total change between two rounds.
        double l1_change = 0.0;

        for (VertexId u = 0; u < n; ++u) {
            l1_change += std::abs(next_rank[u] - rank[u]);
        }

        rank.swap(next_rank);

        result.iterations = iteration + 1;
        result.final_l1_change = l1_change;

        if (l1_change < options.tolerance) {
            result.converged = true;
            break;
        }
    }

    result.scores = std::move(rank);
    return result;
}

} // namespace pr
