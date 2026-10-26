#ifndef PAGERANK_PAGERANK_HPP
#define PAGERANK_PAGERANK_HPP

#include "csr_graph.hpp"

#include <cstddef>
#include <vector>

namespace pr {

struct PageRankOptions {
    double damping = 0.85;
    double tolerance = 1e-10;
    std::size_t max_iterations = 1000;
};

struct PageRankResult {
    std::vector<double> scores;
    std::size_t iterations = 0;
    double final_l1_change = 0.0;
    bool converged = false;
};

PageRankResult compute_pagerank(
    const CSRGraph& graph,
    const PageRankOptions& options = PageRankOptions{}
);

} // namespace pr

#endif
