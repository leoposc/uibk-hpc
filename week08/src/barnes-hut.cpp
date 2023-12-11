#include <iostream>
#include <vector>
#include <cmath>

struct Point {
    float x, y;
    Point() : x(0), y(0) {}
    Point(float x, float y) : x(x), y(y) {}
};

struct Body {
    float mass;
    Point position;
    Point velocity;

    Body(float mass, const Point& position, const Point& velocity)
        : mass(mass), position(position), velocity(velocity) {}
};

class Quad {
public:
    Point center;
    float size;
    bool hasBody;
    Body body;
    Quad* NW;
    Quad* NE;
    Quad* SW;
    Quad* SE;

    Quad(const Point& center, float size)
        : center(center), size(size), hasBody(false), body(), NW(nullptr), NE(nullptr), SW(nullptr), SE(nullptr) {}

    ~Quad() {
        delete NW;
        delete NE;
        delete SW;
        delete SE;
    }
};

class BarnesHut {
public:
    BarnesHut(float theta, float G)
        : theta(theta), G(G), root(nullptr) {}

    ~BarnesHut() {
        delete root;
    }

    void insert(Body body) {
        if (root == nullptr) {
            root = new Quad({0, 0}, 1000); // Adjust the size based on your simulation
        }

        insert(root, body);
    }

    void simulate(float dt) {
        // Your simulation logic here
    }

private:
    float theta;
    const unsigned int G;
    Quad* root;

    void insert(Quad* node, Body body) {
        if (!node->hasBody) {
            node->hasBody = true;
            node->body = body;
            return;
        }

        if (node->size / std::sqrt((body.position.x - node->center.x) * (body.position.x - node->center.x) +
                                  (body.position.y - node->center.y) * (body.position.y - node->center.y)) < theta) {
            // Use center of mass to approximate distant bodies
            float totalMass = node->body.mass + body.mass;
            node->center.x = (node->body.mass * node->center.x + body.mass * body.position.x) / totalMass;
            node->center.y = (node->body.mass * node->center.y + body.mass * body.position.y) / totalMass;
            node->body.mass = totalMass;
            return;
        }

        if (body.position.x < node->center.x) {
            if (body.position.y < node->center.y) {
                if (node->SW == nullptr) {
                    node->SW = new Quad({node->center.x - node->size / 4, node->center.y - node->size / 4}, node->size / 2);
                }
                insert(node->SW, body);
            } else {
                if (node->NW == nullptr) {
                    node->NW = new Quad({node->center.x - node->size / 4, node->center.y + node->size / 4}, node->size / 2);
                }
                insert(node->NW, body);
            }
        } else {
            if (body.position.y < node->center.y) {
                if (node->SE == nullptr) {
                    node->SE = new Quad({node->center.x + node->size / 4, node->center.y - node->size / 4}, node->size / 2);
                }
                insert(node->SE, body);
            } else {
                if (node->NE == nullptr) {
                    node->NE = new Quad({node->center.x + node->size / 4, node->center.y + node->size / 4}, node->size / 2);
                }
                insert(node->NE, body);
            }
        }
    }
};

int main() {
    const unsigned int N = 5000; // Adjust the number of bodies based on your simulation
    const unsigned int T = 100; // Adjust the number of time steps based on your simulation
    const unsigned int G = 1; // Adjust the gravitational constant based on your simulation

    // Initialize the simulation
    BarnesHut simulation(0.5, G); // Adjust theta and G based on your simulation
    simulation.insert(Body(N, {0, 0}, {0, 0})); // Add more bodies as needed

    // Run the simulation
    float dt = 0.01; // Adjust the time step based on your simulation
    for (unsigned int t = 0; t < T; ++t) {
        simulation.simulate(dt);
        // Output or visualize the results
    }

    return 0;
}
