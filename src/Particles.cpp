#include "Particles.h"

#include "Texture.h"


Particle* Particles::Add()
{

    m_particles.resize(m_particles.size() + 1);
    Particle* result = &m_particles.back();
    memset(result, 0, sizeof(Particle));
    return result;

}

void Particles::Draw() const
{

    for (size_t i = 0; i < m_particles.size(); ++i)
    {
        const Particle& particle = m_particles[i];
        if (particle.texture)
        {
            glPushMatrix();

            glTranslatef(particle.position.x, particle.position.y, 0);
            glRotatef(particle.rotation, 0, 0, 1);
            glScalef(particle.scale.x, particle.scale.y, 1);
            glColor(particle.color);
            Render_DrawSprite(*particle.texture, -particle.texture->xSize/2, -particle.texture->ySize/2);
            glPopMatrix();
        }
    }

}

void Particles::Update(float deltaTime)
{

    size_t lastActive = -1;
    for (size_t i = 0; i < m_particles.size(); ++i)
    {
        Particle& particle = m_particles[i];
        if (particle.updateFunction)
        {
            if (particle.updateFunction(particle, deltaTime))
            {
                // Still active, compact list
                ++lastActive;
                if (lastActive < i)
                {
                    m_particles[lastActive] = particle;
                }
            }
        }
    }

    m_particles.resize(lastActive + 1);

}

int Particles::GetNumParticles() const
{
    return static_cast<int>(m_particles.size());
}