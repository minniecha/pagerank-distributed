#ifndef PAGERANK_GRAPH_IO_HPP
#define PAGERANK_GRAPH_IO_HPP

#include "csr_graph.hpp"
#include <string>

namespace pr {

// Read N M followed by M directed edges.
// Vertex IDs must be in [0, N).
CSRGraph load_edge_list(const std::string& path);

} // namespace pr

#endif
