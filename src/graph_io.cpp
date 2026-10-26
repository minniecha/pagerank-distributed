#include "graph_io.hpp"

#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pr {

CSRGraph load_edge_list(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Cannot open graph file: " + path);
    }

    long long n = 0;
    long long m = 0;

    if (!(input >> n >> m) || n < 0 || m < 0) {
        throw std::runtime_error("Invalid header: expected nonnegative N M");
    }

    const auto size_limit = std::numeric_limits<std::size_t>::max();
    if (static_cast<unsigned long long>(n) >= size_limit ||
        static_cast<unsigned long long>(m) > size_limit) {
        throw std::runtime_error("Graph counts are too large");
    }

    const auto vertex_count = static_cast<std::size_t>(n);
    const auto edge_count = static_cast<std::size_t>(m);

    CSRGraph graph;
    graph.row_offsets.assign(vertex_count + 1, 0);

    std::vector<std::pair<VertexId, VertexId>> edges;
    edges.reserve(edge_count);

    // 1. Read edges and count outgoing edges.
    for (std::size_t i = 0; i < edge_count; ++i) {
        long long source = 0;
        long long destination = 0;

        if (!(input >> source >> destination)) {
            throw std::runtime_error(
                "Invalid or missing edge record " + std::to_string(i + 1));
        }

        if (source < 0 || source >= n ||
            destination < 0 || destination >= n) {
            throw std::runtime_error(
                "Vertex ID out of range at edge record " +
                std::to_string(i + 1));
        }

        const auto u = static_cast<VertexId>(source);
        const auto v = static_cast<VertexId>(destination);

        edges.emplace_back(u, v);
        ++graph.row_offsets[u + 1];
    }

    input >> std::ws;
    if (input.bad()) {
        throw std::runtime_error("Error while reading graph file");
    }
    if (!input.eof()) {
        throw std::runtime_error(
            "Extra content after the declared edge records");
    }

    // 2. Prefix sum: turn degree counts into segment boundaries.
    for (std::size_t u = 0; u < vertex_count; ++u) {
        graph.row_offsets[u + 1] += graph.row_offsets[u];
    }

    // 3. Fill destinations, using separate write cursors.
    graph.col_indices.resize(edge_count);
    auto next_position = graph.row_offsets;

    for (const auto& edge : edges) {
        const auto u = edge.first;
        const auto v = edge.second;

        graph.col_indices[next_position[u]] = v;
        ++next_position[u];
    }

    return graph;
}

} // namespace pr
