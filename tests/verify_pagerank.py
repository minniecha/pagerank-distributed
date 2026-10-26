import math
import sys
from pathlib import Path


def load_graph(path):
    tokens = Path(path).read_text().split()
    n, m = map(int, tokens[:2])

    if n <= 0 or m < 0 or len(tokens) != 2 + 2 * m:
        raise ValueError("Invalid graph header or edge count")

    incoming = [[] for _ in range(n)]
    degrees = [0] * n

    for i in range(m):
        u = int(tokens[2 + 2 * i])
        v = int(tokens[3 + 2 * i])

        if not (0 <= u < n and 0 <= v < n):
            raise ValueError("Vertex ID out of range")

        incoming[v].append(u)
        degrees[u] += 1

    return incoming, degrees


def reference_pagerank(incoming, degrees):
    n = len(degrees)
    damping = 0.85
    # Tighter than the C++ stopping threshold.
    tolerance = 1e-13
    scores = [1.0 / n] * n

    for iteration in range(1, 2001):
        dangling = math.fsum(
            scores[u] for u in range(n) if degrees[u] == 0
        )

        base = (1.0 - damping) / n + damping * dangling / n

        # Gather contributions from incoming neighbors.
        updated = [
            base + damping * math.fsum(
                scores[u] / degrees[u] for u in incoming[v]
            )
            for v in range(n)
        ]

        change = math.fsum(
            abs(a - b) for a, b in zip(updated, scores)
        )
        scores = updated

        if change < tolerance:
            return scores, iteration

    raise RuntimeError("Reference PageRank did not converge")


def load_cpp_scores(path, n):
    scores = [None] * n

    for line in Path(path).read_text().splitlines():
        vertex_text, score_text = line.split()
        vertex = int(vertex_text)
        score = float(score_text)

        if not (0 <= vertex < n):
            raise ValueError("Invalid vertex in C++ output")
        if scores[vertex] is not None:
            raise ValueError("Duplicate vertex in C++ output")
        if not math.isfinite(score) or score < 0:
            raise ValueError("Invalid score in C++ output")

        scores[vertex] = score

    if any(score is None for score in scores):
        raise ValueError("Missing vertex in C++ output")

    return scores


def main():
    if len(sys.argv) != 3:
        raise ValueError(
            "Usage: python3 tests/verify_pagerank.py <graph> <cpp-scores>"
        )

    incoming, degrees = load_graph(sys.argv[1])
    reference, iterations = reference_pagerank(incoming, degrees)
    cpp = load_cpp_scores(sys.argv[2], len(degrees))

    errors = [abs(a - b) for a, b in zip(cpp, reference)]
    l1_error = math.fsum(errors)
    max_error = max(errors)
    cpp_sum = math.fsum(cpp)

    print(f"Vertices compared: {len(cpp)}")
    print(f"Reference iterations: {iterations}")
    print(f"C++ score sum: {cpp_sum:.12f}")
    print(f"Reference score sum: {math.fsum(reference):.12f}")
    print(f"L1 error: {l1_error:.6e}")
    print(f"Maximum absolute error: {max_error:.6e}")

    passed = l1_error < 1e-8 and abs(cpp_sum - 1.0) < 1e-10
    print("Verification: PASS" if passed else "Verification: FAIL")
    return 0 if passed else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as error:
        print(f"Error: {error}", file=sys.stderr)
        sys.exit(1)
