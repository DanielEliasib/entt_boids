#ifndef BASE_PROC_HPP
#define BASE_PROC_HPP

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <base_definitions.hpp>
#include <chrono>
#include <cmath>
#include <collision_definitions.hpp>
#include <entt/entt.hpp>
#include <iostream>

struct render_process : entt::process<render_process, std::uint32_t>
{
    using delta_type = std::uint32_t;

    render_process(entt::registry& registry) :
        registry(registry) {}

    const Color border_color = ColorAlpha(BLACK, 0.5);

    void update(delta_type delta_time, void*)
    {
        auto start = std::chrono::high_resolution_clock::now();

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

                // DrawCircleV(vertices[0], 1.4f, GREEN);
                // DrawCircleV(vertices[1], 1.4f, RED);
                // DrawCircleV(vertices[2], 1.6f, BLUE);

                DrawTriangleLines(vertices[0], vertices[1], vertices[2], border_color);
            }
            // else if (renderable.vertices.size() > 3)
            //          {
            //              for (int i = 0; i < renderable.vertices.size(); i++)
            //              {
            //                  DrawLineEx(renderable.vertices[i],
            //                             renderable.vertices[(i + 1) %
            //                                                 renderable.vertices.size()],
            //                             1.0f, renderable.color);
            //              }
            //              for (auto vertex : vertices)
            //              {
            //                  DrawCircleV(vertex, 1.2f, LIGHTGRAY);
            //              }
            //          }
            // DrawCircleV(Vector2Zero(), 1.2f, LIGHTGRAY);
            rlPopMatrix();
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "render_process took " << duration.count() << " microseconds" << std::endl;
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
        auto start = std::chrono::high_resolution_clock::now();

        auto movement_view = registry.view<transform, movement>();
        for (auto [entity, transform, movement] : movement_view.each())
        {
            transform.position = Vector2Add(
                transform.position,
                Vector2Scale(movement.velocity, delta_time / 1000.0f));
            transform.direction = Vector2Normalize(movement.velocity);
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "movement_process took " << duration.count() << " microseconds" << std::endl;
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

struct boids_constraints_process : entt::process<boids_constraints_process, std::uint32_t>
{
    using delta_type = std::uint32_t;

    boids_constraints_process(entt::registry& registry) :
        registry(registry)
    {
        screen_width  = GetScreenWidth();
        screen_height = GetScreenHeight();
    }

    void update(delta_type delta_time, void*)
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto moving_entities_view = registry.view<transform, movement>();

        for (auto [entity, transform_data, movement_data] : moving_entities_view.each())
        {
            Vector2 center_direction = Vector2Subtract(Vector2{screen_width * 0.5f, screen_height * 0.5f}, transform_data.position);
            center_direction         = Vector2Normalize(center_direction);
            bool override_velocity   = false;

            if (transform_data.position.x < 0)
            {
                movement_data.velocity.x *= center_direction.x;
                transform_data.position.x = 0;
            } else if (transform_data.position.x > screen_width)
            {
                movement_data.velocity.x *= center_direction.x;
                transform_data.position.x = screen_width;
            }

            if (transform_data.position.y < 0)
            {
                movement_data.velocity.y *= center_direction.y;
                transform_data.position.y = 0;
            } else if (transform_data.position.y > screen_height)
            {
                movement_data.velocity.y *= center_direction.y;
                transform_data.position.y = screen_height;
            }

            float turnfactor = 1;
            float border     = 50;

            if (transform_data.position.x < border)
            {
                movement_data.velocity.x += turnfactor;
            }

            if (transform_data.position.x > screen_width - border)
            {
                movement_data.velocity.x -= turnfactor;
            }

            if (transform_data.position.y < border)
            {
                movement_data.velocity.y += turnfactor;
            }

            if (transform_data.position.y > screen_height - border)
            {
                movement_data.velocity.y -= turnfactor;
            }

            auto speed = Vector2Length(movement_data.velocity);

            if (speed <= 1)
                movement_data.velocity = center_direction;

            if (!(speed >= min_speed && speed <= max_speed))
            {
                speed                  = std::clamp(speed, min_speed, max_speed);
                movement_data.velocity = Vector2Scale(Vector2Normalize(movement_data.velocity), speed);
            }

            movement_data.old_velocity = movement_data.velocity;
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "boids_constraints_process took " << duration.count() << " microseconds" << std::endl;
    }

   protected:
    entt::registry& registry;
    int screen_width;
    int screen_height;
    float min_speed = 10;
    float max_speed = 70;
};

#endif // BASE_PROC_HPP
