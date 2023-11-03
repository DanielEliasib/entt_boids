#ifndef BASE_PROC_HPP
#define BASE_PROC_HPP

#include <entt.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <base_definitions.hpp>
#include <collision_definitions.hpp>

struct render_process : entt::process<render_process, std::uint32_t> {
    using delta_type = std::uint32_t;

    render_process(entt::registry &registry) : registry(registry) {}

    void update(delta_type delta_time, void *) {
        auto render_view = registry.view<transform, renderable>();
        for (auto [entity, transform, renderable] : render_view.each()) {
            std::vector<Vector2> vertices = renderable.vertices;

            rlPushMatrix();
            float angle =
                atan2(transform.direction.y, transform.direction.x) * RAD2DEG;
            rlTranslatef(transform.position.x, transform.position.y, 0.0f);
            rlRotatef(angle, 0.0f, 0.0f, 1.0f);

            if (renderable.vertices.size() == 3) {
                DrawTriangle(vertices[0], vertices[1], vertices[2],
                             renderable.color);

                DrawCircleV(vertices[0], 1.4f, GREEN);
                DrawCircleV(vertices[1], 1.4f, RED);
                DrawCircleV(vertices[2], 1.6f, BLUE);

                DrawTriangleLines(vertices[0], vertices[1], vertices[2], BLACK);
            } else if (renderable.vertices.size() > 3) {
                for (int i = 0; i < renderable.vertices.size(); i++) {
                    DrawLineEx(renderable.vertices[i],
                               renderable.vertices[(i + 1) %
                                                   renderable.vertices.size()],
                               1.0f, renderable.color);
                }
                for (auto vertex : vertices) {
                    DrawCircleV(vertex, 1.2f, LIGHTGRAY);
                }
            }
            DrawCircleV(Vector2Zero(), 1.2f, LIGHTGRAY);
            rlPopMatrix();
        }
    }

  protected:
    entt::registry &registry;
};

struct movement_process : entt::process<movement_process, std::uint32_t> {
    using delta_type = std::uint32_t;

    movement_process(entt::registry &registry) : registry(registry) {}

    void update(delta_type delta_time, void *) {
        auto movement_view = registry.view<transform, movement>();
        for (auto [entity, transform, movement] : movement_view.each()) {
            transform.position = Vector2Add(
                transform.position,
                Vector2Scale(movement.velocity, delta_time / 1000.0f));
            transform.direction = Vector2Normalize(movement.velocity);
        }
    }

  protected:
    entt::registry &registry;
};

struct vision_process : entt::process<vision_process, std::uint32_t> {
	using delta_type = std::uint32_t;

	vision_process(entt::registry &registry) : registry(registry) {}

	void update(delta_type delta_time, void *) {
		auto boids_view = registry.view<transform, movement>();
		for (auto [entity, transform, movement] : boids_view.each()) {
			RayCollision hit_point;
			if(raycast(registry, transform.position, transform.direction, 100, &hit_point))
			{
				DrawLineEx(transform.position, {hit_point.point.x, hit_point.point.y}, 1.0f, RED);
			}
		}
	}

  protected:
	entt::registry &registry;
};

#endif // BASE_PROC_HPP
