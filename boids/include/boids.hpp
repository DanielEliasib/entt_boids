#ifndef BOIDS_HPP
#define BOIDS_HPP

#include <raylib.h>
#include <raymath.h>

#include <base_definitions.hpp>
#include <boids_definitions.hpp>
#include <entt.hpp>
#include <stack>
#include <vector>

namespace boids
{

    static entt::entity create_boid(entt::registry& registry, Vector2 position,
                                    Vector2 direction, Vector2 velocity, int side,
                                    std::vector<Vector2> original_triangle)
    {
        auto entity = registry.create();

        registry.emplace<transform>(entity, transform{position, direction});
        registry.emplace<movement>(entity, movement{velocity});
        registry.emplace<boid>(entity, boid{-1});
        registry.emplace<renderable>(
            entity,
            renderable(DARKGRAY, static_cast<float>(side), original_triangle));

        return entity;
    }

    static void create_n_boids(entt::registry& registry, int n,
                               Vector2 spawn_position, float spawn_radius)
    {
        auto random_float = [](float min, float max) -> float {
            float random = rand() / (float)RAND_MAX;
            return random * (max - min) + min;
        };

        int side     = 10;
        float height = sqrt(pow(side, 2) - pow(side / 2, 2));

        auto grid = registry.create();
        registry.emplace<boids::grid>(grid, boids::grid(100));
        auto grid_data = registry.get<boids::grid>(grid);

        Vector2 v1 = Vector2{0 - height / 2, 0 - side / 2.0f};
        Vector2 v2 = Vector2{0 - height / 2, 0 + side / 2.0f};
        Vector2 v3 = Vector2{0 + height / 2, 0};

        std::vector<Vector2> original_triangle = {v1, v2, v3};

        for (int i = 0; i < n; i++)
        {
            Vector2 position =
                Vector2{random_float(-spawn_radius / 2, spawn_radius / 2),
                        random_float(-spawn_radius / 2, spawn_radius / 2)};
            position = Vector2Add(position, spawn_position);

            Vector2 direction = Vector2{random_float(-1, 1), random_float(-1, 1)};

            auto hash = grid_data.hash_position(position);

            auto boid =
                create_boid(registry, position, direction,
                            Vector2Scale(direction, 20), side, original_triangle);
            grid_data.add_boid_to_cell(boid, hash);
        }
    }

    // separation process
    struct boid_separation_process : entt::process<boid_separation_process, std::uint32_t>
    {
        using delta_type = std::uint32_t;

        boid_separation_process(entt::registry& registry) :
            registry(registry)
        {
            screen_width  = GetScreenWidth();
            screen_height = GetScreenHeight();
        }

        void update(delta_type delta_time, void*)
        {
            auto boids_view = registry.view<transform, movement, boid>();
            auto grid_view  = registry.view<boids::grid>();
            auto grid_data  = grid_view.get<boids::grid>(grid_view.front());

            std::unordered_set<entt::entity> close_boids;

            for (auto [entity, transform_data, movement_data, boid_data] : boids_view.each())
            {
                grid_data.get_boids_in_cell(boid_data.current_cell_id, close_boids);

                Vector2 separation_force  = Vector2Zero();
                Vector2 average_direction = Vector2Zero();
                Vector2 average_position  = Vector2Zero();

                for (auto close_boid : close_boids)
                {
                    if (close_boid == entity)
                        continue;

                    auto close_boid_transform = boids_view.get<transform>(close_boid);
                    auto close_boid_movement  = boids_view.get<movement>(close_boid);

                    Vector2 direction = Vector2Subtract(transform_data.position, close_boid_transform.position);
                    float distance    = Vector2Length(direction);

                    separation_force = Vector2Add(
                        separation_force, Vector2Scale(Vector2Normalize(direction),
                                                       std::clamp(20.0f / distance, 0.0f, 1.0f)));

                    average_direction = Vector2Add(average_direction, transform_data.direction);
                    average_position  = Vector2Add(average_position, transform_data.position);
                }
                average_direction = close_boids.size() <= 0 ? transform_data.direction : Vector2Normalize(Vector2Scale(average_direction, 1.0f / close_boids.size()));
                average_position  = close_boids.size() <= 0 ? transform_data.position : Vector2Scale(average_position, 1.0f / close_boids.size());

                auto cohercion_direction = Vector2Subtract(average_position, transform_data.position);
                float cohercion_strenght = std::clamp(20.0f / Vector2Length(cohercion_direction), 0.0f, 5.0f);
                cohercion_direction      = Vector2Normalize(cohercion_direction);

                //             Vector2 center_direction = Vector2Subtract(Vector2{screen_width * 0.5f, screen_height * 0.5f}, transform_data.position);
                // center_direction = Vector2Scale(Vector2Normalize(center_direction), 2);

                float separation = Vector2Length(separation_force);
                separation_force = Vector2Normalize(separation_force);
                separation_force = Vector2Add(separation_force, average_direction);
                separation_force = Vector2Add(separation_force, cohercion_direction);
                separation_force = Vector2Scale(separation_force, cohercion_strenght * separation / 3.0f);

                movement_data.velocity = Vector2Add(movement_data.velocity, separation_force);
            }
        }

       protected:
        entt::registry& registry;
        int screen_width  = 0;
        int screen_height = 0;
    };

} // namespace boids

#endif // BOIDS_HPP
