#ifndef BASE_PROC_HPP
#define BASE_PROC_HPP

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <base_definitions.hpp>
#include <cmath>
#include <collision_definitions.hpp>
#include <entt.hpp>

struct render_process : entt::process<render_process, std::uint32_t>
{
    using delta_type = std::uint32_t;

    render_process(entt::registry& registry) :
        registry(registry) {}

    void update(delta_type delta_time, void*)
    {
        auto render_view = registry.view<transform, renderable>();
        for (auto [entity, transform, renderable] : render_view.each())
        {
            std::vector<Vector2> vertices = renderable.vertices;

            rlPushMatrix();
            float angle =
                atan2(transform.direction.y, transform.direction.x) * RAD2DEG;
            rlTranslatef(transform.position.x, transform.position.y, 0.0f);
            rlRotatef(angle, 0.0f, 0.0f, 1.0f);

            if (renderable.vertices.size() == 3)
            {
                DrawTriangle(vertices[0], vertices[1], vertices[2],
                             renderable.color);

                DrawCircleV(vertices[0], 1.4f, GREEN);
                DrawCircleV(vertices[1], 1.4f, RED);
                DrawCircleV(vertices[2], 1.6f, BLUE);

                DrawTriangleLines(vertices[0], vertices[1], vertices[2], BLACK);
            } else if (renderable.vertices.size() > 3)
            {
                for (int i = 0; i < renderable.vertices.size(); i++)
                {
                    DrawLineEx(renderable.vertices[i],
                               renderable.vertices[(i + 1) %
                                                   renderable.vertices.size()],
                               1.0f, renderable.color);
                }
                for (auto vertex : vertices)
                {
                    DrawCircleV(vertex, 1.2f, LIGHTGRAY);
                }
            }
            DrawCircleV(Vector2Zero(), 1.2f, LIGHTGRAY);
            rlPopMatrix();
        }
    }

   protected:
    entt::registry& registry;
};

struct movement_process : entt::process<movement_process, std::uint32_t>
{
    using delta_type = std::uint32_t;

    movement_process(entt::registry& registry) :
        registry(registry) {}

    void update(delta_type delta_time, void*)
    {
        auto movement_view = registry.view<transform, movement>();
        for (auto [entity, transform, movement] : movement_view.each())
        {
            transform.position = Vector2Add(
                transform.position,
                Vector2Scale(movement.velocity, delta_time / 1000.0f));
            transform.direction = Vector2Normalize(movement.velocity);
        }
    }

   protected:
    entt::registry& registry;
};

struct vision_process : entt::process<vision_process, std::uint32_t>
{
    using delta_type = std::uint32_t;

    vision_process(entt::registry& registry) :
        registry(registry) {}

    void update(delta_type delta_time, void*)
    {
        auto boids_view = registry.view<transform, movement>();
        for (auto [entity, transform, movement] : boids_view.each())
        {
            std::vector<RayCollision> hit_points;
            if (raycast(registry, transform.position, transform.direction, hit_points, 100))
            {
                RayCollision closest_hit = hit_points[0];
                DrawLineEx(transform.position,
                           {closest_hit.point.x, closest_hit.point.y}, 1.0f, RED);
            }
        }
    }

   protected:
    entt::registry& registry;
};

struct wall_collision_process : entt::process<wall_collision_process, std::uint32_t>
{
    using delta_type = std::uint32_t;

    wall_collision_process(entt::registry& registry) :
        registry(registry)
    {
        screen_width  = GetScreenWidth();
        screen_height = GetScreenHeight();
    }

    void update(delta_type delta_time, void*)
    {
        auto moving_entities_view = registry.view<transform, movement>();

        // INFO: Simple wall collision, can't figure out collisions
        for (auto [entity, transform_data, movement_data] : moving_entities_view.each())
        {
			if (transform_data.position.x < 0)
			{
				movement_data.velocity.x *= -1;
				transform_data.position.x = 0;
			} else if (transform_data.position.x > screen_width)
			{
				movement_data.velocity.x *= -1;
				transform_data.position.x = screen_width;
			}

			if (transform_data.position.y < 0)
			{
				movement_data.velocity.y *= -1;
				transform_data.position.y = 0;
			} else if (transform_data.position.y > screen_height)
			{
				movement_data.velocity.y *= -1;
				transform_data.position.y = screen_height;
			}
        }
    }

   protected:
    entt::registry& registry;
    int screen_width;
    int screen_height;
};

#endif // BASE_PROC_HPP
