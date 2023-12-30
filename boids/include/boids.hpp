#ifndef BOIDS_HPP
#define BOIDS_HPP

#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <base_definitions.hpp>
#include <boids_definitions.hpp>
#include <cmath>
#include <entt/entt.hpp>
#include <execution>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace boids
{

    static const Color boid_color = {225, 225, 225, 255};
    static entt::entity create_boid(entt::registry& registry, Vector2 position,
                                    Vector2 direction, Vector2 velocity, int side,
                                    std::vector<Vector2> original_triangle, int id)
    {
        auto entity = registry.create();

        registry.emplace<transform>(entity, transform{position, direction});
        registry.emplace<movement>(entity, movement{velocity, velocity});
        registry.emplace<boid>(entity, boid{-1, id});
        registry.emplace<renderable>(
            entity,
            renderable(boid_color, static_cast<float>(side), original_triangle));

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
        registry.emplace<boids::grid>(grid, boids::grid(40));
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
            auto start = std::chrono::high_resolution_clock::now();

            auto boids_view = registry.view<transform, movement, boid>();
            auto grid_view  = registry.view<boids::grid>();
            auto grid_data  = grid_view.get<boids::grid>(grid_view.front());

            float separation_radius = 30.0f;
            float cohesion_radius   = 80.0f;

            Vector2 target_pos = GetMousePosition();

            // TODO: remove unecesarry operation already calcualted in grid data process

            auto parallel_func = [&](auto& entity) {
                transform& transform_data = registry.get<transform>(entity);
                movement& movement_data   = registry.get<movement>(entity);
                boid& boid_data           = registry.get<boid>(entity);

                std::unordered_set<entt::entity> close_boids;

                int cohesion_boids_count        = 0;
                Vector2 local_cohesion_center   = Vector2{0, 0};
                Vector2 local_cohesion_velocity = Vector2{0, 0};

                int separation_boids_count    = 0;
                Vector2 local_sepation_center = Vector2{0, 0};

                auto cell_id     = boid_data.current_cell_id;
                auto close_cells = grid_data.get_close_cells(cell_id);
                close_cells.push_back(cell_id);

                for (auto id : close_cells)
                {
                    std::unordered_set<entt::entity> cell_boids;
                    grid_data.get_boids_in_cell(id, cell_boids);

                    close_boids.insert(cell_boids.begin(), cell_boids.end());
                }

                close_boids.erase(entity);

                for (auto close_boid : close_boids)
                {
                    auto close_boid_transform = registry.get<transform>(close_boid);
                    auto close_boid_movement  = registry.get<movement>(close_boid);

                    auto close_boid_distance_squared = Vector2DistanceSqr(close_boid_transform.position, transform_data.position);

                    if (close_boid_distance_squared < separation_radius * separation_radius)
                    {
                        local_sepation_center = Vector2Add(local_sepation_center, close_boid_transform.position);

                        separation_boids_count++;
                    }

                    if (close_boid_distance_squared < cohesion_radius * cohesion_radius)
                    {
                        local_cohesion_center   = Vector2Add(local_cohesion_center, close_boid_transform.position);
                        local_cohesion_velocity = Vector2Add(local_cohesion_velocity, close_boid_movement.old_velocity);

                        cohesion_boids_count++;
                    }

                    if (boid_data.id == debug_boid_id)
                    {
                        DrawLineEx(transform_data.position, close_boid_transform.position, 2, LIME);
                    }
                }

                Vector2 cohesion_force   = Vector2Zero();
                Vector2 separation_force = Vector2Zero();

                if (cohesion_boids_count > 0)
                {
                    local_cohesion_center   = Vector2Scale(local_cohesion_center, 1.0f / cohesion_boids_count);
                    local_cohesion_velocity = Vector2Scale(local_cohesion_velocity, 1.0f / cohesion_boids_count);

                    cohesion_force = Vector2Scale(Vector2Normalize(Vector2Subtract(local_cohesion_center, transform_data.position)), 30);
                }
                local_cohesion_velocity = Vector2Normalize(local_cohesion_velocity);

                if (separation_boids_count > 0)
                {
                    local_sepation_center = Vector2Scale(local_sepation_center, 1.0f / separation_boids_count);

                    separation_force = Vector2Scale(Vector2Normalize(Vector2Subtract(transform_data.position, local_sepation_center)), 60);
                }

                Vector2 temp             = Vector2Subtract(target_pos, transform_data.position);
                float distance_to_target = Vector2Length(temp);
                float target_scale       = std::clamp(distance_to_target, 0.0f, cohesion_radius) * 5.0f / cohesion_radius;

                Vector2 target_force = Vector2Scale(temp, target_scale / distance_to_target);

                // Vector2 alignment_force = Vector2Scale(Vector2Normalize(Vector2Subtract(local_flock_direction, Vector2Normalize(movement_data.velocity))), 50);
                Vector2 alignment_force = Vector2Scale(local_cohesion_velocity, 15);

                Vector2 total_force = Vector2Add(alignment_force, separation_force);
                total_force         = Vector2Add(total_force, cohesion_force);
                total_force         = Vector2Add(total_force, target_force);

                // add random noise to the total force
                float x = GetRandomValue(-100, 100) * (5 / 100.0f);
                float y = GetRandomValue(-100, 100) * (5 / 100.0f);

                total_force = Vector2Add(total_force, Vector2{x, y});

                movement_data.velocity = Vector2Add(movement_data.old_velocity, Vector2Scale(total_force, delta_time / 1000.0f));

                //---------------------------------------------
                if (debug_boid_id == boid_data.id)
                {
                    DrawCircleLinesV(transform_data.position, separation_radius, ColorAlpha(GREEN, 0.6f));
                    DrawCircleLinesV(transform_data.position, cohesion_radius, ColorAlpha(RED, 0.6f));

                    auto grid_view   = registry.view<grid>();
                    auto grid_entity = grid_view.front();

                    if (grid_entity == entt::null)
                        return;

                    auto& grid_data = registry.get<grid>(grid_entity);

                    for (auto id : close_cells)
                    {
                        auto [x, y] = grid_data.cell_id_to_index(id);

                        Color color = !grid_data.is_cell_empty(id)
                                          ? RED
                                          : ColorAlpha(GRAY, 0.3f);

                        DrawRectangleLines(x * grid_data.cell_size, y * grid_data.cell_size,
                                           grid_data.cell_size, grid_data.cell_size, color);
                    }
                    // DrawCircleV(target_pos, 5, VIOLET);
                    //
                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, alignment_force), BLUE);
                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, separation_force), GREEN);
                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, cohesion_force), RED);
                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, target_force), VIOLET);
                    //
                    DrawLineV(transform_data.position, Vector2Add(transform_data.position, total_force), BLACK);
                }
                //---------------------------------------------
            };

            std::for_each(std::execution::par, boids_view.begin(), boids_view.end(), parallel_func);

            auto end      = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "boid_algo_process took " << duration.count() << " microseconds" << std::endl;
        }

       protected:
        entt::registry& registry;

        int debug_boid_id = 0;

        int screen_width  = 0;
        int screen_height = 0;
    };

} // namespace boids

#endif // BOIDS_HPP
