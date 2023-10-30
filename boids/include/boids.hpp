#pragma once

#include <boids_definitions.hpp>
#include <entt.hpp>
#include <raylib.h>
#include <raymath.h>
#include <stack>
#include <vector>

namespace boids {

static void create_boid(entt::registry &registry, Vector2 position,
                        Vector2 direction, Vector2 velocity, int side, std::vector<Vector2> original_triangle) {

    auto entity = registry.create();

    registry.emplace<transform>(entity, transform{position, direction});
    registry.emplace<movement>(entity, movement{velocity});
    registry.emplace<renderable>(entity,
                                 renderable(DARKGRAY, static_cast<float>(side), original_triangle));
}

static void create_n_boids(entt::registry &registry, int n,
                           Vector2 spawn_position, float spawn_radius) {
    auto random_float = [](float min, float max) -> float {
        float random = rand() / (float)RAND_MAX;
        return random * (max - min) + min;
    };

	int side = 10;
    float height = sqrt(pow(side, 2) - pow(side / 2, 2));

	Vector2 v1 = Vector2{0 - height/2, 0 - side/2.0f};
	Vector2 v2 = Vector2{0 - height/2, 0 + side/2.0f};
	Vector2 v3 = Vector2{0 + height / 2, 0};
	
	std::vector<Vector2> original_triangle = {v1, v2, v3};

    for (int i = 0; i < n; i++) {

        Vector2 position =
            Vector2{random_float(-spawn_radius / 2, spawn_radius / 2),
                    random_float(-spawn_radius / 2, spawn_radius / 2)};
        position = Vector2Add(position, spawn_position);

        Vector2 direction = Vector2{random_float(-1, 1), random_float(-1, 1)};

        create_boid(registry, position, direction, Vector2Scale(direction, 10), side, original_triangle);
    }
}
} // namespace boids
