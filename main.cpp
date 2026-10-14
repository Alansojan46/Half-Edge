#include <iostream>
#include <utility>
#include <vector>
#include <Eigen/Dense>

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

std::pair<int, int> Find(
    int start,
    int end,
    const std::vector<Eigen::Vector3i>& triangles
) {
    for (int i = 0; i < triangles.size(); ++i) {

        const Eigen::Vector3i& Fi = triangles[i];

        for (int j = 0; j < 3; ++j) {

            int a = Fi[j];
            int b = Fi[(j + 1) % 3];

            if (a == start && b == end) {
                return {i, j};
            }
        }
    }

    return {-1, -1};
}

int main() {

    std::vector<Eigen::Vector3d> points = {
        {0, 0,  0},
        {1, 0,  0},
        {1, 1,  0},
        {0, 1,  0},
        {0, 0, -1},
        {1, 0, -1},
        {1, 1, -1},
        {0, 1, -1}
    };

    std::vector<Eigen::Vector3i> triangles = {
        {0, 1, 2},
        {0, 2, 3},
        {1, 5, 6},
        {1, 6, 2},
        {4, 7, 5},
        {5, 7, 6},
        {0, 3, 4},
        {3, 7, 4},
        {2, 6, 7},
        {2, 7, 3},
        {0, 4, 1},
        {1, 4, 5}
    };

    std::vector<Vertex*> TabVertex;
    std::vector<Face*> TabFace;
    std::vector<HalfEdge*> TabHalfEdge;

    for (int i = 0; i < points.size(); ++i) {

        Vertex* p = new Vertex;

        p->position = points[i];
        p->he = nullptr;

        TabVertex.push_back(p);
    }

    for (int i = 0; i < triangles.size(); ++i) {

        Eigen::Vector3i Fi = triangles[i];

        Face* tmpFace = new Face;

        HalfEdge* he0 = new HalfEdge;
        HalfEdge* he1 = new HalfEdge;
        HalfEdge* he2 = new HalfEdge;

        tmpFace->he = he0;
        TabFace.push_back(tmpFace);

        if (TabVertex[Fi[0]]->he == nullptr)
            TabVertex[Fi[0]]->he = he0;

        if (TabVertex[Fi[1]]->he == nullptr)
            TabVertex[Fi[1]]->he = he1;

        if (TabVertex[Fi[2]]->he == nullptr)
            TabVertex[Fi[2]]->he = he2;

        he0->twin = nullptr;
        he0->target = TabVertex[Fi[1]];
        he0->next = he1;
        he0->face = tmpFace;

        TabHalfEdge.push_back(he0);

        he1->twin = nullptr;
        he1->target = TabVertex[Fi[2]];
        he1->next = he2;
        he1->face = tmpFace;

        TabHalfEdge.push_back(he1);

        he2->twin = nullptr;
        he2->target = TabVertex[Fi[0]];
        he2->next = he0;
        he2->face = tmpFace;

        TabHalfEdge.push_back(he2);
    }

    for (int i = 0; i < triangles.size(); ++i) {

        const Eigen::Vector3i& Fi = triangles[i];

        for (int j = 0; j < 3; ++j) {

            int start = Fi[j];
            int end = Fi[(j + 1) % 3];

            std::pair<int, int> result =
                Find(end, start, triangles);

            if (result.first != -1) {

                int twinFace = result.first;
                int twinLocalEdge = result.second;

                HalfEdge* h =
                    TabHalfEdge[3 * i + j];

                HalfEdge* opposite =
                    TabHalfEdge[3 * twinFace + twinLocalEdge];

                h->twin = opposite;
            }
        }
    }

    std::cout << "===== HALF-EDGE CUBE =====\n";

    std::cout << "Vertices: "
              << TabVertex.size() << "\n";

    std::cout << "Faces: "
              << TabFace.size() << "\n";

    std::cout << "Half-edges: "
              << TabHalfEdge.size() << "\n\n";

    std::cout << "===== TWIN CONNECTIONS =====\n";

    int missingTwins = 0;
    int wrongTwins = 0;

    for (int i = 0; i < triangles.size(); ++i) {

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

    int invalidFaces = 0;

    for (int i = 0; i < TabFace.size(); ++i) {

        Face* f = TabFace[i];

        HalfEdge* h0 = f->he;
        HalfEdge* h1 = h0->next;
        HalfEdge* h2 = h1->next;

        const Eigen::Vector3i& Fi = triangles[i];

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

    int invalidVertices = 0;

    for (Vertex* v : TabVertex) {

        bool outgoing =
            v->he != nullptr &&
            v->he->next->next->target == v;

        if (!outgoing)
            ++invalidVertices;
    }

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

    for (HalfEdge* h : TabHalfEdge)
        delete h;

    for (Face* f : TabFace)
        delete f;

    for (Vertex* v : TabVertex)
        delete v;

    return 0;
}
