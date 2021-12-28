#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        masses.push_back(new Mass(start, node_mass, false));
        for (int n = 1; n < num_nodes-1; n++) {
            masses.push_back(new Mass(start + n * (end - start)/(num_nodes - 1), node_mass, false));
            springs.push_back(new Spring(masses[n-1], masses[n], k));
        }
        masses.push_back(new Mass(end, node_mass, false));
        springs.push_back(new Spring(masses[num_nodes-2], masses[num_nodes-1], k));
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    // gravity是重力加速度
    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D mLength = s->m2->position - s->m1->position;
            Vector2D force = s->k * mLength.unit() * (mLength.norm() - s->rest_length);
            s->m1->forces += force;
            s->m2->forces -= force;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                // TODO (Part 2): Add global damping
                m->forces += gravity * m->mass;
                const float kd = 0.01f;
                m->forces += -kd * m->velocity;

                Vector2D accelerate = m->forces / m->mass;
                m->velocity += accelerate * delta_t;
                m->position += m->velocity * delta_t;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D mLength = s->m2->position - s->m1->position;
            Vector2D force = s->k * mLength.unit() * (mLength.norm() - s->rest_length);
            s->m1->forces += force;
            s->m2->forces -= force;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D tempPosition = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                // TODO (Part 4): Add global Verlet damping
                m->forces += gravity * m->mass;
                Vector2D accelerate = m->forces / m->mass;

                const float dampingFactor = 0.00005f;
                m->position = m->position + (1 - dampingFactor) * (m->position - m->last_position) + accelerate * delta_t * delta_t;

                m->last_position = tempPosition;
            }
        }
    }
}
