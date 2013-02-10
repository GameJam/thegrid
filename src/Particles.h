#ifndef GAME_PARTICLES_H
#define GAME_PARTICLES_H

#include "Vec2.h"

#include <vector>

struct Particle;
struct Texture;

typedef bool (*UpdateFunction)(Particle& particle, float deltaTime);

struct Particle
{
    float           time;
    Vec2            position;
    Vec2            scale;
    float           rotation;

    Texture*        texture;
    UpdateFunction  updateFunction;

};

class Particles
{
public:

    void Draw() const;
    Particle* Add();
    void Update(float deltaTime);

private:
    std::vector<Particle> m_particles;
};

#endif
