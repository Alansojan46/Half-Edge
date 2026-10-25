
#include <iostream>
#include <utility>
#include <vector>
#include <Eigen/Dense>
#include <GL/freeglut.h>

class Vertex;
class Face;
class HalfEdge;

class Vertex {
public:
    Eigen::Vector3d position;
    HalfEdge* he = nullptr;
};

class Face {
public:
    HalfEdge* he = nullptr;
};

class HalfEdge {
public:
    Vertex* target = nullptr;
    HalfEdge* twin = nullptr;
    HalfEdge* next = nullptr;
    Face* face = nullptr;
};

class mesh {
private:
    std::vector<Eigen::Vector3d> points;
    std::vector<Eigen::Vector3i> triangles;

    std::vector<Vertex*> TabVertex;
    std::vector<Face*> TabFace;
    std::vector<HalfEdge*> TabHalfEdge;

public:
    mesh(const std::vector<Eigen::Vector3d>& points,
         const std::vector<Eigen::Vector3i>& triangles)
        : points(points), triangles(triangles) {

        initializeVertices();
        initializeFacesAndHalfEdges();
        connectTwins();
    }

    mesh(const mesh&) = delete;
    mesh& operator=(const mesh&) = delete;

    ~mesh() {

        for (int i = 0; i < TabHalfEdge.size(); ++i) {
            delete TabHalfEdge[i];
        }

        for (int i = 0; i < TabFace.size(); ++i) {
            delete TabFace[i];
        }

        for (int i = 0; i < TabVertex.size(); ++i) {
            delete TabVertex[i];
        }
    }

    
    void drawPoints() const {

        glPointSize(12.0f);
        glColor3f(1.0f, 0.85f, 0.15f);

        glBegin(GL_POINTS);

        for (int i = 0; i < TabVertex.size(); ++i) {

        const Eigen::Vector3d& p = TabVertex[i]->position;

        glVertex3d(p.x(), p.y(), p.z());
    }

        glEnd();
    }

    
    void drawHalfEdges() const {

        glLineWidth(2.5f);
        glColor3f(0.1f, 0.85f, 1.0f);

        glBegin(GL_LINES);

        for (int i = 0; i < TabHalfEdge.size(); ++i) {

            HalfEdge* h = TabHalfEdge[i];

        if (h == h->face->he) {
            glColor3f(1.0f, 0.0f, 0.0f);
        } else {
            glColor3f(0.1f, 0.85f, 1.0f);
        }


            const Eigen::Vector3d& a = h->next->next->target->position;
            const Eigen::Vector3d& b = h->target->position;
            const Eigen::Vector3d& c = h->next->target->position;

            Eigen::Vector3d direction = b - a;
            double length = direction.norm();

            if (length == 0.0)
                continue;

            direction /= length;

            Eigen::Vector3d inward = c - (a + b) * 0.5;
            inward -= inward.dot(direction) * direction;

            if (inward.norm() == 0.0)
                continue;

            inward.normalize();

            Eigen::Vector3d offset = 0.055 * inward;

            Eigen::Vector3d from = a + 0.15 * (b - a) + offset;
            Eigen::Vector3d to = a + 0.85 * (b - a) + offset;

            Eigen::Vector3d arrowLeft = to - 0.11 * length * direction
                                    + 0.055 * length * inward;

            Eigen::Vector3d arrowRight = to - 0.11 * length * direction
                                     - 0.055 * length * inward;

            glVertex3dv(from.data());
            glVertex3dv(to.data());

            glVertex3dv(to.data());
            glVertex3dv(arrowLeft.data());

            glVertex3dv(to.data());
            glVertex3dv(arrowRight.data());
    }

    glEnd();
}



    void verify() const {

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

                HalfEdge* h = TabHalfEdge[3 * i + j];

                int start = Fi[j];
                int end = Fi[(j + 1) % 3];

                std::cout << "Face " << i
                          << ", he" << j
                          << " (" << start
                          << " -> " << end << ")";

                if (h->twin == nullptr) {
                    ++missingTwins;
                    std::cout << " | twin: NOT FOUND\n";
                    continue;
                }

                std::pair<int, int> twinPosition =
                    Find(end, start);

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

                std::cout << " | twin: Face "
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

        for (int i = 0; i < TabVertex.size(); ++i) {

            Vertex* v = TabVertex[i];

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
    }


    
    void drawFaces() const {

        const Eigen::Vector3d lightDirection =
            Eigen::Vector3d(0.5, 0.8, 1.0).normalized();

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        glBegin(GL_TRIANGLES);

        for (int i = 0; i < TabFace.size(); ++i) {

            HalfEdge* h0 = TabFace[i]->he;
            HalfEdge* h1 = h0->next;
            HalfEdge* h2 = h1->next;

            const Eigen::Vector3d& a = h2->target->position;
            const Eigen::Vector3d& b = h0->target->position;
            const Eigen::Vector3d& c = h1->target->position;

            Eigen::Vector3d normal = (b - a).cross(c - a);

            if (normal.norm() == 0.0)
                continue;

            normal.normalize();

            double diffuse = normal.dot(lightDirection);

            if (diffuse < 0.0)
                diffuse = 0.0;

            double grey = 0.42 + 0.33 * diffuse;

            glColor3d(grey, grey, grey);

            glVertex3dv(a.data());
            glVertex3dv(b.data());
            glVertex3dv(c.data());
        }

        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
}


private:
    std::pair<int, int> Find(int start, int end) const {

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

    void initializeVertices() {

        for (int i = 0; i < points.size(); ++i) {

            Vertex* p = new Vertex;

            p->position = points[i];
            p->he = nullptr;

            TabVertex.push_back(p);
        }
    }

    void initializeFacesAndHalfEdges() {

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
    }

    void connectTwins() {

        for (int i = 0; i < triangles.size(); ++i) {

            const Eigen::Vector3i& Fi = triangles[i];

            for (int j = 0; j < 3; ++j) {

                int start = Fi[j];
                int end = Fi[(j + 1) % 3];

                std::pair<int, int> result =
                    Find(end, start);

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
    }
};



mesh* cubeOnScreen = nullptr;


double yaw = 0.7;
double pitch = 0.35;
double cameraDistance = 3.5;

bool dragging = false;

int lastMouseX = 0;
int lastMouseY = 0;




void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double centerX = 0.5;
    double centerY = 0.5;
    double centerZ = -0.5;

    double cameraX = centerX +
        cameraDistance * cos(pitch) * sin(yaw);

    double cameraY = centerY +
        cameraDistance * sin(pitch);

    double cameraZ = centerZ +
        cameraDistance * cos(pitch) * cos(yaw);

    gluLookAt(
        cameraX, cameraY, cameraZ,
        centerX, centerY, centerZ,
        0.0, 1.0, 0.0
    );
    
    if (cubeOnScreen != nullptr) {

        cubeOnScreen->drawFaces();
        cubeOnScreen->drawHalfEdges();
        cubeOnScreen->drawPoints();
    }


    glutSwapBuffers();
}


void mouse(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON) {

        if (state == GLUT_DOWN) {
            dragging = true;
            lastMouseX = x;
            lastMouseY = y;
        }

        if (state == GLUT_UP) {
            dragging = false;
        }
    }
}

void motion(int x, int y) {

    if (dragging) {

        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        yaw += dx * 0.01;
        pitch -= dy * 0.01;

        if (pitch > 1.5)
            pitch = 1.5;

        if (pitch < -1.5)
            pitch = -1.5;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

void mouseWheel(int wheel, int direction, int x, int y) {

    if (direction > 0) {
        cameraDistance *= 0.9;
    }

    if (direction < 0) {
        cameraDistance *= 1.1;
    }

    if (cameraDistance < 1.0)
        cameraDistance = 1.0;

    if (cameraDistance > 15.0)
        cameraDistance = 15.0;

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {

    if (key == 'r' || key == 'R') {

        yaw = 0.7;
        pitch = 0.35;
        cameraDistance = 3.5;

        glutPostRedisplay();
    }
}



void reshape(int width, int height) {

    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0,
        double(width) / height,
        0.1,
        100.0
    );

    glMatrixMode(GL_MODELVIEW);
}


int main(int argc, char** argv) {

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

    mesh cube(points, triangles);
    cube.verify();

    cubeOnScreen = &cube;

    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH
    );

    glutInitWindowSize(800, 600);

    glutCreateWindow("Half-Edge Cube - Vertices");

    glClearColor(0.07f, 0.09f, 0.15f, 1.0f);

    glEnable(GL_DEPTH_TEST);


    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMouseWheelFunc(mouseWheel);
    glutKeyboardFunc(keyboard);

    glutMainLoop();



    return 0;
}

