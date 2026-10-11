
#include <iostream>
#include <utility>
#include <vector>
#include <Eigen/Dense>

// ============================================
// 1. HALF-EDGE DATA STRUCTURES
// ============================================

struct Vertex;
struct Face;
struct HalfEdge;

struct Vertex {
    Eigen::Vector3d position;
    HalfEdge* he = nullptr;
};

struct Face {
    HalfEdge* he = nullptr;
};

struct HalfEdge {
    Vertex* target = nullptr;
    HalfEdge* twin = nullptr;
    HalfEdge* next = nullptr;
    Face* face = nullptr;
};

// ============================================
// 2. FIND FUNCTION (INEFFICIENT SEARCH)
// ============================================

// Search for a directed edge start -> end.
//
// Returns:
//   first  = face index
//   second = local half-edge index (0, 1, 2)
//
// If not found, returns {-1, -1}.

std::pair<int, int> Find(
    int start,
    int end,
    const std::vector<Eigen::Vector3i>& triangles
) {

    // Examine every face
    for (int i = 0;
         i < static_cast<int>(triangles.size());
         ++i) {

        const Eigen::Vector3i& Fi = triangles[i];

        // Examine the three edges of the face
        for (int j = 0; j < 3; ++j) {

            int a = Fi[j];
            int b = Fi[(j + 1) % 3];

            if (a == start && b == end) {
                return {i, j};
            }
        }
    }

    // Edge not found
    return {-1, -1};
}

// ============================================
// 3. MAIN FUNCTION
// ============================================

int main() {

    // ========================================
    // 3.1. DEFINE CUBE VERTICES
    // ========================================

    std::vector<Eigen::Vector3d> points = {
        {0, 0,  0}, // p0
        {1, 0,  0}, // p1
        {1, 1,  0}, // p2
        {0, 1,  0}, // p3
        {0, 0, -1}, // p4
        {1, 0, -1}, // p5
        {1, 1, -1}, // p6
        {0, 1, -1}  // p7
    };

    // ========================================
    // 3.2. DEFINE TRIANGULAR FACES
    // ========================================

    std::vector<Eigen::Vector3i> triangles = {
        {0, 1, 2}, // Face 0
        {0, 2, 3}, // Face 1
        {1, 5, 6}, // Face 2
        {1, 6, 2}, // Face 3
        {4, 7, 5}, // Face 4
        {5, 7, 6}, // Face 5
        {0, 3, 4}, // Face 6
        {3, 7, 4}, // Face 7
        {2, 6, 7}, // Face 8
        {2, 7, 3}, // Face 9
        {0, 4, 1}, // Face 10
        {1, 4, 5}  // Face 11
    };

    // ========================================
    // 4. CREATE THREE POINTER TABLES
    // ========================================

    std::vector<Vertex*> TabVertex;
    std::vector<Face*> TabFace;
    std::vector<HalfEdge*> TabHalfEdge;

    // ========================================
    // 5. INITIALIZE VERTICES
    // ========================================

    for (int i = 0;
         i < static_cast<int>(points.size());
         ++i) {

        Vertex* p = new Vertex;

        p->position = points[i];
        p->he = nullptr;

        TabVertex.push_back(p);
    }

    // ========================================
    // 6. INITIALIZE FACES AND HALF-EDGES
    // ========================================

    for (int i = 0;
         i < static_cast<int>(triangles.size());
         ++i) {

        Eigen::Vector3i Fi = triangles[i];

        // Create one face
        Face* tmpFace = new Face;

        // Create three half-edges
        HalfEdge* he0 = new HalfEdge;
        HalfEdge* he1 = new HalfEdge;
        HalfEdge* he2 = new HalfEdge;

        // Initialize face
        tmpFace->he = he0;
        TabFace.push_back(tmpFace);

        // Initialize one outgoing half-edge
        // for each vertex

        if (TabVertex[Fi[0]]->he == nullptr)
            TabVertex[Fi[0]]->he = he0;

        if (TabVertex[Fi[1]]->he == nullptr)
            TabVertex[Fi[1]]->he = he1;

        if (TabVertex[Fi[2]]->he == nullptr)
            TabVertex[Fi[2]]->he = he2;

        // ------------------------------------
        // Half-edge 0: Fi[0] -> Fi[1]
        // ------------------------------------

        he0->twin = nullptr;
        he0->target = TabVertex[Fi[1]];
        he0->next = he1;
        he0->face = tmpFace;

        TabHalfEdge.push_back(he0);

        // ------------------------------------
        // Half-edge 1: Fi[1] -> Fi[2]
        // ------------------------------------

        he1->twin = nullptr;
        he1->target = TabVertex[Fi[2]];
        he1->next = he2;
        he1->face = tmpFace;

        TabHalfEdge.push_back(he1);

        // ------------------------------------
        // Half-edge 2: Fi[2] -> Fi[0]
        // ------------------------------------

        he2->twin = nullptr;
        he2->target = TabVertex[Fi[0]];
        he2->next = he0;
        he2->face = tmpFace;

        TabHalfEdge.push_back(he2);
    }

    // ========================================
    // 7. FIND AND CONNECT THE TWINS
    // ========================================

    for (int i = 0;
         i < static_cast<int>(triangles.size());
         ++i) {

        const Eigen::Vector3i& Fi = triangles[i];

        // Examine three edges of Face i
        for (int j = 0; j < 3; ++j) {

            // Current directed edge
            int start = Fi[j];
            int end = Fi[(j + 1) % 3];

            // Search for its reverse:
            // end -> start

            std::pair<int, int> result =
                Find(end, start, triangles);

            if (result.first != -1) {

                // Face and local edge of the twin
                int twinFace = result.first;
                int twinLocalEdge = result.second;

                // Current half-edge pointer
                HalfEdge* h =
                    TabHalfEdge[3 * i + j];

                // Twin half-edge pointer
                HalfEdge* opposite =
                    TabHalfEdge[
                        3 * twinFace + twinLocalEdge
                    ];

                // Connect the twin
                h->twin = opposite;
            }
        }
    }

    // ========================================
    // 8. VERIFY THE RESULTS ON SCREEN
    // ========================================

    std::cout << "===== HALF-EDGE CUBE =====\n";

    std::cout << "Vertices: "
              << TabVertex.size() << "\n";

    std::cout << "Faces: "
              << TabFace.size() << "\n";

    std::cout << "Half-edges: "
              << TabHalfEdge.size() << "\n\n";

    // ========================================
    // 8.1. VERIFY TWIN CONNECTIONS
    // ========================================

    std::cout << "===== TWIN CONNECTIONS =====\n";

    int missingTwins = 0;
    int wrongTwins = 0;

    for (int i = 0;
         i < static_cast<int>(triangles.size());
         ++i) {

        const Eigen::Vector3i& Fi = triangles[i];

        for (int j = 0; j < 3; ++j) {

            HalfEdge* h =
                TabHalfEdge[3 * i + j];

            int start = Fi[j];
            int end = Fi[(j + 1) % 3];

            std::cout << "Face " << i
                      << ", he" << j
                      << " (" << start
                      << " -> " << end << ")";

            if (h->twin == nullptr) {

                ++missingTwins;

                std::cout
                    << " | twin: NOT FOUND\n";

                continue;
            }

            std::pair<int, int> twinPosition =
                Find(end, start, triangles);

            // Verify reciprocal twin pointers
            // and opposite endpoint directions

            bool valid =
                twinPosition.first >= 0 &&

                h->twin ==
                TabHalfEdge[
                    3 * twinPosition.first +
                    twinPosition.second
                ] &&

                h->twin->twin == h &&

                h->target ==
                h->twin->next->next->target &&

                h->next->next->target ==
                h->twin->target;

            if (!valid)
                ++wrongTwins;

            std::cout
                << " | twin: Face "
                << twinPosition.first
                << ", he"
                << twinPosition.second
                << " | "
                << (valid ? "OK" : "ERROR")
                << "\n";
        }
    }

    // ========================================
    // 8.2. VERIFY FACE CONNECTIONS
    // ========================================

    int invalidFaces = 0;

    for (int i = 0;
         i < static_cast<int>(TabFace.size());
         ++i) {

        Face* f = TabFace[i];

        HalfEdge* h0 = f->he;
        HalfEdge* h1 = h0->next;
        HalfEdge* h2 = h1->next;

        const Eigen::Vector3i& Fi =
            triangles[i];

        bool valid =
            h2->next == h0 &&

            h0->face == f &&
            h1->face == f &&
            h2->face == f &&

            h0->target == TabVertex[Fi[1]] &&
            h1->target == TabVertex[Fi[2]] &&
            h2->target == TabVertex[Fi[0]];

        if (!valid)
            ++invalidFaces;
    }

    // ========================================
    // 8.3. VERIFY VERTEX CONNECTIONS
    // ========================================

    int invalidVertices = 0;

    for (Vertex* v : TabVertex) {

        bool outgoing =
            v->he != nullptr &&
            v->he->next->next->target == v;

        if (!outgoing)
            ++invalidVertices;
    }

    // ========================================
    // 8.4. FINAL SUMMARY
    // ========================================

    std::cout << "\n===== SUMMARY =====\n";

    std::cout << "Missing twins: "
              << missingTwins << "\n";

    std::cout << "Incorrect twins: "
              << wrongTwins << "\n";

    std::cout << "Incorrect faces: "
              << invalidFaces << "\n";

    std::cout << "Incorrect vertex half-edges: "
              << invalidVertices << "\n";

    if (missingTwins == 0 &&
        wrongTwins == 0 &&
        invalidFaces == 0 &&
        invalidVertices == 0) {

        std::cout << "All checks passed!\n";

    } else {

        std::cout << "Some checks failed.\n";
    }

    // ========================================
    // 9. FREE DYNAMIC MEMORY
    // ========================================

    for (HalfEdge* h : TabHalfEdge)
        delete h;

    for (Face* f : TabFace)
        delete f;

    for (Vertex* v : TabVertex)
        delete v;

    return 0;
}

