#ifndef PAGERANK_CSR_GRAPH_HPP
#define PAGERANK_CSR_GRAPH_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace pr {

using VertexId = std::size_t;
using EdgeOffset = std::size_t;

// Outgoing-edge CSR.
//
// Valid graph invariants:
// 1. row_offsets contains N + 1 elements and starts with 0.
// 2. row_offsets is nondecreasing.
// 3. row_offsets.back() equals col_indices.size().
// 4. Every destination vertex is in [0, N).
//
// The future graph builder will check these invariants.
struct CSRGraph {
    // {0} represents a valid graph with zero vertices and zero edges.
    std::vector<EdgeOffset> row_offsets{0};
    std::vector<VertexId> col_indices;

    [[nodiscard]] std::size_t num_vertices() const noexcept {
        return row_offsets.size() - 1;
    }

    [[nodiscard]] std::size_t num_edges() const noexcept {
        return col_indices.size();
    }

    [[nodiscard]] std::size_t out_degree(VertexId vertex) const {
        if (vertex >= num_vertices()) {
            throw std::out_of_range("CSRGraph: vertex ID out of range");
        }

        return row_offsets[vertex + 1] - row_offsets[vertex];
    }
};

} // namespace pr

#endif
