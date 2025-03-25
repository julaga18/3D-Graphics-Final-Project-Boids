#ifndef BOID_HPP
#define BOID_HPP

#include <glm.hpp>
#include <vector>

// Definicje sta�ych

const float MAX_SPEED = 3.0f;       // Maksymalna pr�dko��
const float NEIGHBOR_RADIUS = 5.0f;  // Promie� s�siedztwa
const float AVOID_RADIUS = 2.0f;    // Promie� unikania

const float SCREEN_WIDTH = 10.0f*0.4f;   // Szeroko�� przestrzeni
const float SCREEN_HEIGHT = 10.0f * 0.4f;  // Wysoko�� przestrzeni
const float SCREEN_DEPTH = 10.0f * 1.2f;   // G��boko�� przestrzeni

extern float ALIGNMENT_WEIGHT; // Waga zgodno�ci
extern float COHESION_WEIGHT;  // Waga sp�jno�ci
extern float SEPARATION_WEIGHT; // Waga separacji

class Boid {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::mat4 rotationMatrix;
    int group;

    // Konstruktor
    Boid(glm::vec3 position, glm::vec3 velocity, int group);
    
    // Metody
    glm::vec3 align(const std::vector<Boid>& boids);
    glm::vec3 cohesion(const std::vector<Boid>& boids);
    glm::vec3 separation(const std::vector<Boid>& boids);


    void update(std::vector<Boid>& boids, float deltaTime);
    void updateRotation();  // <--- Nowa funkcja obrotu
};

#endif // BOID_HPP
