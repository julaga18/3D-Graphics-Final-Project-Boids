#include "Boid.hpp"
#include <vector>

// Zmienne zewnêtrzne
float ALIGNMENT_WEIGHT = 1.0f;
float COHESION_WEIGHT = 1.0f;
float SEPARATION_WEIGHT = 1.0f;


Boid::Boid(glm::vec3 pos, glm::vec3 vel, int group)
    : position(pos), velocity(vel), group(group) {}


glm::vec3 Boid::align(const std::vector<Boid>& boids) {
    glm::vec3 avgVelocity(0.0f);
    int count = 0;

    for (const Boid& boid : boids) {
        if (boid.position == this->position || boid.group != this->group) {
            continue;
        }
        if (glm::distance(this->position, boid.position) < NEIGHBOR_RADIUS) {
            avgVelocity += boid.velocity;
            count++;
        }
    }

    if (count > 0) {
        avgVelocity /= static_cast<float>(count);
        if (glm::length(avgVelocity) > 0.0f) {  // Unikamy normalizacji (0,0,0)
            avgVelocity = glm::normalize(avgVelocity) * MAX_SPEED;
        }
        return avgVelocity - velocity;
    }

    return glm::vec3(0.0f);
}

glm::vec3 Boid::cohesion(const std::vector<Boid>& boids) {
    glm::vec3 center_of_group(0.0f, 0.0f, 0.0f);
    int count = 0;

    for (const Boid& boid : boids) {
        // Pomijamy siebie i boidy z innych grup
        if (&boid == this || boid.group != this->group)
            continue;

        // Jeœli w zasiêgu NEIGHBOR_RADIUS
        float distance = glm::distance(this->position, boid.position);
        if (distance < NEIGHBOR_RADIUS) {
            center_of_group += boid.position;
            count++;
        }
    }

    if (count > 0) {
        center_of_group /= count;  // Œrodek grupy
        return glm::normalize(center_of_group - this->position); // Zwrócenie wektora do œrodka grupy
    }

    return glm::vec3(0.0f); // Brak innych boidów w zasiêgu
}


glm::vec3 Boid::separation(const std::vector<Boid>& boids) {
    glm::vec3 avoid_vector(0.0f, 0.0f, 0.0f);

    for (const Boid& boid : boids) {
        if (&boid == this || boid.group != this->group)
            continue;

        float distance = glm::distance(this->position, boid.position);
        if (distance < AVOID_RADIUS) {
            glm::vec3 diff = this->position - boid.position;
            avoid_vector += glm::normalize(diff) / distance;
        }
    }

    return avoid_vector;
}


void Boid::update(std::vector<Boid>& boids, float deltaTime) {
    // Obliczamy alignment, cohesion, separation
    glm::vec3 alignment = align(boids) * ALIGNMENT_WEIGHT;
    glm::vec3 cohesion_force = cohesion(boids) * COHESION_WEIGHT;
    glm::vec3 separation_force = separation(boids) * SEPARATION_WEIGHT;

    // Sumujemy wszystkie si³y
    glm::vec3 steering = alignment + cohesion_force + separation_force;

    // Dodajemy wektor kierunku do velocity
    velocity += steering;


    // Jeœli prêdkoœæ przekracza MAX_SPEED, normalizujemy j¹
    if (glm::length(velocity) > MAX_SPEED) {
        velocity = glm::normalize(velocity) * MAX_SPEED;
    }

    // Przemieszczamy pozycjê boida
    position += velocity * deltaTime;

    updateRotation();  //Aktualizacja rotacji

    // Poprawione odbijanie od granic
    if (position.x > SCREEN_WIDTH) {
        position.x = SCREEN_WIDTH;
        velocity.x *= -1;
    }
    if (position.x < -SCREEN_WIDTH) {
        position.x = -SCREEN_WIDTH;
        velocity.x *= -1;
    }

    if (position.y > SCREEN_HEIGHT) {
        position.y = SCREEN_HEIGHT;
        velocity.y *= -1;
    }
    if (position.y < -SCREEN_HEIGHT) {
        position.y = -SCREEN_HEIGHT;
        velocity.y *= -1;
    }

    if (position.z > SCREEN_DEPTH) {
        position.z = SCREEN_DEPTH;
        velocity.z *= -1;
    }
    if (position.z < -SCREEN_DEPTH) {
        position.z = -SCREEN_DEPTH;
        velocity.z *= -1;
    }
}

    //void Boid::updateRotation() {
    //    if (glm::length(velocity) > 0.2f) { // Sprawdzenie, czy boid siê rusza
    //        glm::vec3 forward = glm::normalize(velocity);
    //        glm::vec3 up = glm::vec3(0, 1, 0);
    // 
    //        glm::vec3 right = glm::normalize(glm::cross(up, forward));
    //        up = glm::cross(forward, right);

    //        rotationMatrix = glm::mat4(1.0f);
    //        rotationMatrix[0] = glm::vec4(right, 0.0f);
    //        rotationMatrix[1] = glm::vec4(up, 0.0f);
    //        rotationMatrix[2] = glm::vec4(forward, 0.0f);
    //    }

    //}

    void Boid::updateRotation() {
        if (glm::length(velocity) > 0.2f) {
            glm::vec3 forward = glm::normalize(velocity);
            glm::vec3 up = glm::vec3(0, 1, 0);

            // Zapobiega degeneracji up, jeœli forward jest prawie pionowy
            if (glm::abs(glm::dot(forward, up)) > 0.99f) {
                up = glm::vec3(1, 0, 0);
            }

            glm::vec3 right = glm::normalize(glm::cross(up, forward));
            up = glm::cross(forward, right);

            rotationMatrix = glm::mat4(1.0f);
            rotationMatrix[0] = glm::vec4(right, 0.0f);
            rotationMatrix[1] = glm::vec4(up, 0.0f);
            rotationMatrix[2] = glm::vec4(forward, 0.0f);
        }
    }







  
