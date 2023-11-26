#ifndef BOIDS_DEF_HPP
#define BOIDS_DEF_HPP

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <entt/entt.hpp>
#include <iostream>
#include <unordered_set>

#include "base_definitions.hpp"
#include "collision_definitions.hpp"

namespace boids
{

    struct boid
    {
        int current_cell_id;
    };

    struct grid
    {
        int cell_size;
        int window_width;
        int window_height;
        int cell_count;

        std::unordered_map<int, std::unordered_set<entt::entity>>
            cell_to_boids;

        grid(int cell_size) :
            cell_size(cell_size)
        {
            window_width  = GetScreenWidth();
            window_height = GetScreenHeight();
            cell_count    = (window_width / cell_size) * (window_height / cell_size);
            cell_to_boids = std::unordered_map<int, std::unordered_set<entt::entity>>();
        }

        int hash_position(Vector2 position)
        {
            int x      = static_cast<int>(floor(position.x));
            int y      = static_cast<int>(floor(position.y));
            int cell_x = x / cell_size;
            int cell_y = y / cell_size;

            return cell_x + cell_y * (window_width / cell_size); // 2D to 1D
        }

        std::vector<int> get_close_cells(int cell_id)
        {
            auto check_func = [&](int x, int y) {
                if (x < 0 || x >= window_width / cell_size)
                    return false;
                if (y < 0 || y >= window_height / cell_size)
                    return false;
                return true;
            };

            auto ids    = std::vector<int>();
            auto [x, y] = cell_id_to_index(cell_id);

            int n_x = x;
            int n_y = y - 1;
            if (check_func(n_x, n_y))
                ids.push_back(n_x + n_y * (window_width / cell_size));

            n_x = x;
            n_y = y + 1;
            if (check_func(n_x, n_y))
                ids.push_back(n_x + n_y * (window_width / cell_size));

            n_x = x - 1;
            n_y = y;
            if (check_func(n_x, n_y))
                ids.push_back(n_x + n_y * (window_width / cell_size));

            n_x = x + 1;
            n_y = y;
            if (check_func(n_x, n_y))
                ids.push_back(n_x + n_y * (window_width / cell_size));

            return ids;
        }
        // grid cell id to 2D index
        std::pair<int, int> cell_id_to_index(int cell_id)
        {
            int x = cell_id % (window_width / cell_size);
            int y = cell_id / (window_width / cell_size);

            return {x, y};
        }

        bool is_boid_in_cell(entt::entity entity, int cell_id)
        {
            if (cell_to_boids.find(cell_id) == cell_to_boids.end())
                return false;

            return cell_to_boids[cell_id].find(entity) !=
                   cell_to_boids[cell_id].end();
        }

        bool is_cell_empty(int cell_id)
        {
            if (cell_to_boids.find(cell_id) == cell_to_boids.end())
                return true;

            return cell_to_boids[cell_id].size() == 0;
        }

        void add_boid_to_cell(entt::entity entity, int cell_id)
        {
            if (cell_to_boids.find(cell_id) == cell_to_boids.end())
            {
                auto set               = std::unordered_set<entt::entity>();
                cell_to_boids[cell_id] = set;
            }

            cell_to_boids[cell_id].insert(entity);
        }

        void remove_boid_from_cell(entt::entity entity, int cell_id)
        {
            if (cell_to_boids.find(cell_id) == cell_to_boids.end())
                return;

            cell_to_boids[cell_id].erase(entity);
        }

        void get_boids_in_cell(int cell_id, std::unordered_set<entt::entity>& boids)
        {
            if (cell_to_boids.find(cell_id) == cell_to_boids.end())
                return;

            boids = cell_to_boids[cell_id];
        }
    };

    struct collision_avoidance_process : entt::process<collision_avoidance_process, std::uint32_t>
    {
        using delta_type = std::uint32_t;

        collision_avoidance_process(entt::registry& registry) :
            registry(registry) {}

        void update(delta_type delta_time, void*)
        {
            auto boids_view = registry.view<transform, movement>();
            for (auto [entity, transform_data, movement_data] : boids_view.each())
            {
                std::vector<RayCollision> hit_points;
                if (raycast(registry, transform_data.position,
                            transform_data.direction, hit_points, 75))
                {
                    RayCollision collision_point = hit_points[0];

                    auto normal = collision_point.normal;
                    auto v      = Vector3CrossProduct(normal, {0, 0, 1});

                    auto dot = Vector3DotProduct(v, {transform_data.direction.x, transform_data.direction.y, 0});
                    if (dot < 0)
                    {
                        v = Vector3Scale(v, -1);
                    }

                    Vector2 half_vector = Vector2Add(Vector2{normal.x, normal.y}, Vector2{v.x, v.y});
                    half_vector         = Vector2Normalize(half_vector);

                    float current_speed = Vector2Length(movement_data.velocity);
                    current_speed       = current_speed <= 5.0f
                                              ? 5.0f
                                              : current_speed; // TODO: REMOVE HARDCODING

                    Vector2 target_direction = Vector2Lerp(
                        transform_data.direction,
                        half_vector,
                        delta_time * 0.0001f * 10);
                    target_direction = Vector2Normalize(target_direction);

                    movement_data.velocity =
                        Vector2Scale(target_direction, current_speed);
                }
            }
        }

       protected:
        entt::registry& registry;
    };

    struct boid_hashing_process : entt::process<boid_hashing_process, std::uint32_t>
    {
        using delta_type = std::uint32_t;

        boid_hashing_process(entt::registry& registry) :
            registry(registry) {}

        void update(delta_type delta_time, void*)
        {
            auto boids_view  = registry.view<transform, movement, boid>();
            auto grid_view   = registry.view<grid>();
            auto grid_entity = grid_view.front();

            if (grid_entity == entt::null)
                return;

            auto& grid_data = registry.get<grid>(grid_entity);

            for (auto [entity, transform_data, movement_data, boid_data] :
                 boids_view.each())
            {
                auto hash = grid_data.hash_position(transform_data.position);

                if (boid_data.current_cell_id != -1)
                {
                    grid_data.remove_boid_from_cell(entity, boid_data.current_cell_id);
                }
                grid_data.add_boid_to_cell(entity, hash);
                boid_data.current_cell_id = hash;

                // if (boid_data.current_cell_id != hash) {
                //     grid_data.remove_boid_from_cell(entity,
                //                                     boid_data.current_cell_id);
                //     grid_data.add_boid_to_cell(entity, hash);
                //
                //     boid_data.current_cell_id = hash;
                //     return;
                // }
                //
                // if (!grid_data.is_boid_in_cell(entity, hash)) {
                //     grid_data.add_boid_to_cell(entity, hash);
                //     boid_data.current_cell_id = hash;
                // }
            }
        }

       protected:
        entt::registry& registry;
    };

    struct cell_renderer_process : entt::process<cell_renderer_process, std::uint32_t>
    {
        using delta_type = std::uint32_t;

        cell_renderer_process(entt::registry& registry) :
            registry(registry) {}

        void update(delta_type delta_time, void*)
        {
            auto grid_view   = registry.view<grid>();
            auto grid_entity = grid_view.front();

            if (grid_entity == entt::null)
                return;

            auto& grid_data = registry.get<grid>(grid_entity);

            for (int cell_id = 0; cell_id < grid_data.cell_count; cell_id++)
            {
                auto [x, y] = grid_data.cell_id_to_index(cell_id);

                Color color = !grid_data.is_cell_empty(cell_id)
                                  ? RED
                                  : ColorAlpha(GRAY, 0.1f);

                DrawRectangleLines(x * grid_data.cell_size, y * grid_data.cell_size,
                                   grid_data.cell_size, grid_data.cell_size, color);
            }
        }

       protected:
        entt::registry& registry;
    };

} // namespace boids

#endif // BOIDS_DEF_HPP
