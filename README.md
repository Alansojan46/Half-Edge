# Half-Edge Data Structure — Implementation and Optimization - Soft Eçngineering Course - VIBOT M1 - Université Bourgogne Europe

This repository is a C++ learning project exploring **different implementations of the half-edge data structure**, with a particular focus on how to find and connect **twin half-edges** efficiently.

We will start with a simple, deliberately inefficient implementation and progressively introduce faster approaches. The first version serves as a **baseline** for understanding the data structure, checking correctness, and comparing time complexity.

## Version 1 — Brute-force twin search (O(F²))

The current implementation constructs a half-edge representation of a **triangulated cube** and links opposite half-edges using a straightforward linear search.

It uses three structures:

- `Vertex`: stores a 3D position and a pointer to one outgoing half-edge.
- `Face`: stores a pointer to one of its half-edges.
- `HalfEdge`: stores pointers to its target vertex, twin, next half-edge, and incident face.

For each triangle, the program creates three half-edges and links them into a cycle through their `next` pointers.

### How twin search works

For every directed edge `start -> end`, the program calls `Find(end, start, triangles)` to locate its reverse edge:

1. Visit the triangles one by one.
2. Inspect the three directed edges of each triangle.
3. Return the face index and local edge index if the reverse edge is found.
4. Use those indices to connect the `twin` pointer. If no reverse edge exists, the pointer remains `nullptr` (as on a boundary).

### Why is the complexity O(F²)?

Let **F** be the number of triangular faces.

- Each triangle has exactly **3 half-edges**, so there are **3F half-edges**.
- The `Find()` function may inspect all **F faces** (three edges per face), taking **O(F)** time in the worst case.
- We run `Find()` once for each of the **3F half-edges**.

Therefore, the **worst-case time complexity for twin construction** is:

**O(3F × 3F) = O(F²)**.

The subsequent validation also uses `Find()` and has **O(F²)** worst-case time complexity. Creating the vertices, faces, and half-edges takes **O(V + F)** time, where V is the number of vertices. The half-edge representation uses **O(V + F)** memory.

This approach is easy to understand and implement, but it does not scale well to large meshes.

## Current test: triangulated cube

The example mesh contains:

| Element | Count |
| --- | ---: |
| Vertices | 8 |
| Triangular faces | 12 |
| Half-edges | 36 |

The program prints the twin connections and checks that:

- Twin pointers are reciprocal and connect opposite directed edges.
- Each face's three half-edges form a closed cycle.
- Face and vertex pointers are consistent.

For this closed, consistently oriented cube, every half-edge should have a twin.

## Planned implementations

| Version | Twin-search strategy | Expected time complexity |
| --- | --- | --- |
| 1 — Current | Brute-force linear search over all triangles | **O(F²)** worst case |
| 2 — Planned | Sort directed edges and find reverse-edge pairs | **O(F log F)** |
| 3 — Planned | Hash table indexed by directed vertex pairs | **O(F)** expected |

The O(F) hash-based estimate assumes average constant-time hash-table operations; its worst case can be worse.

Our goal is to compare **correctness, runtime, and implementation complexity** across these versions while building a deeper understanding of half-edge meshes.

## Requirements

- A C++ compiler supporting **C++11 or later**
- [Eigen](https://eigen.tuxfamily.org/) for `Eigen::Vector3d` and `Eigen::Vector3i`

Compile the current C++ source file (for example, `main.cpp`) with Eigen's include directory configured. For instance:

```bash
g++ -std=c++17 main.cpp -I /path/to/eigen3 -o half_edge
./half_edge
```

Replace `/path/to/eigen3` with the correct Eigen include path on your machine.

## Project status

**Version 1: brute-force implementation.** Future versions will optimize the twin-search step while preserving the same half-edge structure and correctness checks.
