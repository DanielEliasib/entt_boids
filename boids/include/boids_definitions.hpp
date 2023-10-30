#pragma once

#include <cstdint>
#include <entt.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

namespace boids {

struct transform {
    Vector2 position;
    Vector2 direction;
};

struct movement {
    Vector2 velocity;
};

struct renderable {
    Color color;
    float size;

    std::vector<Vector2> vertices;

    renderable(Color color, float size, std::vector<Vector2> &vertices)
        : color(color), size(size) {
        this->vertices = vertices;
    }
};

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
} // namespace boids
