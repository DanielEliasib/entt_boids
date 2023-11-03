#ifndef BOIDS_DEF_HPP
#define BOIDS_DEF_HPP

#include "base_definitions.hpp"
#include "collision_definitions.hpp"
#include <entt.hpp>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <unordered_set>

namespace boids {

struct boid {
    int current_cell_id;
};

struct collision_avoidance_process
    : entt::process<collision_avoidance_process, std::uint32_t> {
    using delta_type = std::uint32_t;

    collision_avoidance_process(entt::registry &registry)
        : registry(registry) {}

    void update(delta_type delta_time, void *) {
        auto boids_view = registry.view<transform, movement>();
        for (auto [entity, transform_data, movement_data] : boids_view.each()) {
            RayCollision collision_point;
            auto hit = raycast(registry, transform_data.position,
                               transform_data.direction, 30, &collision_point);

            if (hit) {
                float current_speed = Vector2Length(movement_data.velocity);
                current_speed = current_speed <= 5.0f
                                    ? 5.0f
                                    : current_speed; // TODO: REMOVE HARDCODING
                Vector2 target_direction = Vector2Lerp(
                    transform_data.direction,
                    Vector2{collision_point.normal.x, collision_point.normal.y},
                    delta_time * 0.0001f * 20);
                target_direction = Vector2Normalize(target_direction);

                movement_data.velocity =
                    Vector2Scale(target_direction, current_speed);
            }
        }
    }

  protected:
    entt::registry &registry;
};

struct boid_hashing_process
    : entt::process<boid_hashing_process, std::uint32_t> {
    using delta_type = std::uint32_t;

    boid_hashing_process(
        entt::registry &registry,
        std::unordered_map<std::uint64_t, std::unordered_set<entt::entity>>* cell_to_boids,
        int cell_size)
        : registry(registry), cells_map(cell_to_boids), cell_size(cell_size) {
        window_width = GetScreenWidth();
        window_height = GetScreenHeight();
    }

    std::uint64_t hash_position(Vector2 position) {
        int x = static_cast<int>(position.x);
        int y = static_cast<int>(position.y);
        int cell_x = x / cell_size;
        int cell_y = y / cell_size;

        return cell_x + cell_y * (window_width / cell_size); // 2D to 1D
    }

    void update(delta_type delta_time, void *) {
        auto boids_view = registry.view<transform, movement, boid>();
        for (auto [entity, transform_data, movement_data, boid_data] :
             boids_view.each()) {

            auto hash = hash_position(transform_data.position);

            // check if cellID_to_boids[hash] exists
            if (cells_map->find(hash) == cells_map->end()) {
                auto set = std::unordered_set<entt::entity>();
                cells_map->insert({hash, set});
			}

			if(boid_data.current_cell_id == -1){
				(*cells_map)[hash].insert(entity);
				boid_data.current_cell_id = hash;
				return;
			}

            if (boid_data.current_cell_id != hash) {
                ((*cells_map)[boid_data.current_cell_id]).erase(entity);
                (*cells_map)[hash].insert(entity);

                boid_data.current_cell_id = hash;

                return;
            }
			if((*cells_map)[hash].find(entity) == (*cells_map)[hash].end())
			{
				(*cells_map)[hash].insert(entity);
				boid_data.current_cell_id = hash;
			}
        }
    }

  protected:
    entt::registry &registry;
    std::unordered_map<std::uint64_t, std::unordered_set<entt::entity>>*
        cells_map;

    int cell_size = 10;
    int window_width = 800;
    int window_height = 600;
};

struct cell_renderer_process
    : entt::process<cell_renderer_process, std::uint32_t> {
    using delta_type = std::uint32_t;

    cell_renderer_process(
        entt::registry &registry,
        std::unordered_map<std::uint64_t, std::unordered_set<entt::entity>>* cell_to_boids,
        int cell_size)
        : registry(registry), cells_map(cell_to_boids), cell_size(cell_size) {
        window_width = GetScreenWidth();
        window_height = GetScreenHeight();
    }

	void update(delta_type delta_time, void *) 
	{
		for(int i = 0; i < window_width / cell_size; i++)
		{
			for(int j = 0; j < window_height / cell_size; j++)
			{
				int cell_id = i + j * (window_width / cell_size);

				bool is_boid_in_cell = cells_map->find(cell_id) != cells_map->end();
				if(is_boid_in_cell){
					is_boid_in_cell = (*cells_map)[cell_id].size() > 0;
				}
				Color color = is_boid_in_cell ? GREEN : ColorAlpha(GRAY, 0.1f);
					
				DrawRectangleLines(i * cell_size, j * cell_size, cell_size, cell_size, color);
			}
		}
	}

  protected:
    entt::registry &registry;
    std::unordered_map<std::uint64_t, std::unordered_set<entt::entity>>*
        cells_map;

    int cell_size = 10;
    int window_width = 800;
    int window_height = 600;
};

} // namespace boids

#endif // BOIDS_DEF_HPP
