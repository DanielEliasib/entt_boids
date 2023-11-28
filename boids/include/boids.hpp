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
                                    std::vector<Vector2> original_triangle, int id)
    {
        auto entity = registry.create();

        registry.emplace<transform>(entity, transform{position, direction});
        registry.emplace<movement>(entity, movement{velocity});
        registry.emplace<boid>(entity, boid{-1, id});
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
                            Vector2Scale(direction, 20), side, original_triangle, i);
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

            float separation_radius = 60.0f;
            float cohesion_radius   = 90.0f;

            Vector2 target_pos = GetMousePosition();

            std::unordered_set<entt::entity> close_boids;

            // TODO: remove unecesarry operation already calcualted in grid data process

            for (auto [entity, transform_data, movement_data, boid_data] : boids_view.each())
            {
                auto grid_pre_process_data = grid_data.get_cell_data(boid_data.current_cell_id);

                close_boids.clear();
                grid_data.get_boids_in_cell(boid_data.current_cell_id, close_boids);
                int n_close_boids = close_boids.size() - 1;

                auto local_flock_center    = grid_pre_process_data.local_boids_center;
                auto local_flock_direction = grid_pre_process_data.local_boids_direction;
                auto local_boid_count      = grid_pre_process_data.boids_count;

                if (n_close_boids > 0)
                {
                    local_flock_center = Vector2Subtract(local_flock_center, transform_data.position);
                    local_flock_center = Vector2Scale(local_flock_center, 1.0f / (local_boid_count - 1.0f));

                    local_flock_direction = Vector2Subtract(local_flock_direction, transform_data.direction);
                    local_flock_direction = Vector2Scale(local_flock_direction, 1.0f / (local_boid_count - 1.0f));
                } else
                {
                    local_flock_center    = Vector2Zero();
                    local_flock_direction = transform_data.direction;
                }

                std::vector<Vector2> debug_coheision_boids;
                std::vector<Vector2> debug_separation_boids;

				Vector2 local_flock_center_separation = local_flock_center;
                Vector2 local_flock_center_cohesion = Vector2Subtract(local_flock_center, target_pos);

                Vector2 cohesion_force   = Vector2Scale(Vector2Normalize(Vector2Subtract(local_flock_center_cohesion, transform_data.position)), 35);
                Vector2 separation_force = Vector2Scale(Vector2Normalize(Vector2Subtract(transform_data.position, local_flock_center_separation)), 60);

                // Vector2 alignment_force = Vector2Scale(Vector2Normalize(Vector2Subtract(local_flock_direction, Vector2Normalize(movement_data.velocity))), 50);
                Vector2 alignment_force = Vector2Scale(local_flock_direction, 50);
                Vector2 total_force     = Vector2Add(alignment_force, separation_force);
                total_force             = Vector2Add(total_force, cohesion_force);
                //---------------------------------------------
                if (debug_boid_id == boid_data.id)
                {
                    DrawCircleLinesV(transform_data.position, separation_radius, ColorAlpha(GREEN, 0.3f));
                    DrawCircleLinesV(transform_data.position, cohesion_radius, ColorAlpha(RED, 0.3f));

                    // for (auto debug_coheision_boid : debug_coheision_boids)
                    // {
                    //     DrawLineV(transform_data.position, debug_coheision_boid, RED);
                    // }

                    // for (auto debug_separation_boid : debug_separation_boids)
                    // {
                    //     DrawLineV(transform_data.position, debug_separation_boid, GREEN);
                    // }
                    DrawCircleV(target_pos, 5, VIOLET);

                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, alignment_force), BLUE);
                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, separation_force), GREEN);
                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, cohesion_force), RED);
                }

                //---------------------------------------------
                movement_data.velocity = Vector2Add(movement_data.velocity, Vector2Scale(total_force, delta_time / 1000.0f));
            }
        }

       protected:
        entt::registry& registry;

        int debug_boid_id = 0;

        int screen_width  = 0;
        int screen_height = 0;
    };

} // namespace boids

#endif // BOIDS_HPP
