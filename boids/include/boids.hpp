#ifndef BOIDS_HPP
#define BOIDS_HPP

#include <raylib.h>
#include <raymath.h>

#include <base_definitions.hpp>
#include <boids_definitions.hpp>
#include <entt/entt.hpp>
#include <stack>
#include <unordered_set>
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
    struct boid_algo_process : entt::process<boid_algo_process, std::uint32_t>
    {
        using delta_type = std::uint32_t;

        boid_algo_process(entt::registry& registry) :
            registry(registry)
        {
            // WARN: We assume that the screen size is not going to change
            screen_width  = GetScreenWidth();
            screen_height = GetScreenHeight();
        }

        void update(delta_type delta_time, void*)
        {
            auto boids_view = registry.view<transform, movement, boid>();
            auto grid_view  = registry.view<boids::grid>();
            auto grid_data  = grid_view.get<boids::grid>(grid_view.front());

            float separation_radius = 30.0f;
            float cohesion_radius   = 70.0f;

            std::unordered_set<entt::entity> close_boids;

            for (auto [entity, transform_data, movement_data, boid_data] : boids_view.each())
            {
                close_boids.clear();
                grid_data.get_boids_in_cell(boid_data.current_cell_id, close_boids);

                Vector2 local_flock_center_separation = Vector2Zero();
                int n_separation_boids                = 0;

                Vector2 local_flock_center_cohesion = Vector2Zero();
                int n_cohesion_boids                = 0;

                Vector2 local_flock_direction = Vector2Zero();

                for (auto close_boid : close_boids)
                {
                    if (close_boid == entity)
                        continue;

                    auto close_boid_transform = boids_view.get<transform>(close_boid);
                    auto close_boid_movement  = boids_view.get<movement>(close_boid);

                    Vector2 close_boid_direction = Vector2Normalize(close_boid_movement.velocity);

                    float sqr_distance = Vector2DistanceSqr(transform_data.position, close_boid_transform.position);

                    if (sqr_distance <= separation_radius * separation_radius)
                    {
                        local_flock_center_separation = Vector2Add(local_flock_center_separation, close_boid_transform.position);
                        n_separation_boids++;
                    }

                    if (sqr_distance <= cohesion_radius * cohesion_radius)
                    {
                        local_flock_center_cohesion = Vector2Add(local_flock_center_cohesion, close_boid_transform.position);
                        n_cohesion_boids++;
                    }

                    local_flock_direction = Vector2Add(local_flock_direction, close_boid_direction);
                }

                int n_close_boids = close_boids.size() - 1;

                local_flock_center_separation = n_separation_boids > 0 ? Vector2Scale(local_flock_center_separation, 1.0f / (close_boids.size() - 1.0f)) : Vector2Zero();
                local_flock_center_cohesion   = n_cohesion_boids > 0 ? Vector2Scale(local_flock_center_cohesion, 1.0f / (close_boids.size() - 1.0f)) : Vector2Zero();

                local_flock_direction = n_close_boids > 0 ? Vector2Scale(local_flock_direction, 1.0f / (close_boids.size() - 1.0f)) : Vector2Zero();

                Vector2 cohesion_force   = Vector2Scale(Vector2Normalize(Vector2Subtract(local_flock_center_cohesion, transform_data.position)), 0.6f);
                Vector2 separation_force = Vector2Scale(Vector2Normalize(Vector2Subtract(transform_data.position, local_flock_center_separation)), 0.35f);

                Vector2 alignment_force = Vector2Scale(Vector2Normalize(Vector2Subtract(local_flock_direction, Vector2Normalize(movement_data.velocity))), 0.95f);
                Vector2 total_force     = Vector2Add(Vector2Add(cohesion_force, separation_force), alignment_force);
                //---------------------------------------------
                movement_data.velocity = Vector2Add(movement_data.velocity, Vector2Scale(total_force, delta_time / 1000.0f));
            }
        }

       protected:
        entt::registry& registry;
        int screen_width  = 0;
        int screen_height = 0;
    };

} // namespace boids

#endif // BOIDS_HPP
