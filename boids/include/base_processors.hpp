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

struct collision_process : entt::process<collision_process, std::uint32_t>
{
    using delta_type = std::uint32_t;

    collision_process(entt::registry& registry) :
        registry(registry) {}

    void update(delta_type delta_time, void*)
    {
        auto moving_entities_view = registry.view<transform, movement>();

        auto collision_entities_view =
            registry.view<transform, rect_collider>();

        for (auto [moving_entity, transform_data, movement_data] :
             moving_entities_view.each())
        {
            std::vector<Vector2> collision_answers;

            for (auto [collider_entity, collider_transform_data,
                       collider_data] : collision_entities_view.each())
            {
                if (moving_entity == collider_entity)
                {
                    continue;
                }

                float rotation_angle =
                    atan2(collider_transform_data.direction.y,
                          collider_transform_data.direction.x);

                Matrix rot_mat = MatrixRotateZ(rotation_angle);
                // Vector2 transformed_position = Vector2Subtract(
                //     transform_data.position,
                //     collider_transform_data.position);
                // transformed_position = Vector2Transform(transformed_position,
                //                                         MatrixInvert(rot_mat));
                Matrix translation_matrix =
                    MatrixTranslate(collider_transform_data.position.x,
                                    collider_transform_data.position.y, 0);
                Matrix transform_matrix =
                    MatrixMultiply(rot_mat, translation_matrix);

                Rectangle collider_rect = {-collider_data.size.x * 0.5f,
                                           -collider_data.size.y * 0.5f,
                                           collider_data.size.x,
                                           collider_data.size.y};

                Vector2 transformed_position = Vector2Subtract(
                    transform_data.position, collider_transform_data.position);
                transformed_position = Vector2Transform(transformed_position,
                                                        MatrixInvert(rot_mat));

                // Rect starts not at the centerbut at a corner
                if (CheckCollisionPointRec(transformed_position,
                                           collider_rect))
                {
                    Vector2 direction = Vector2Scale(
                        Vector2Normalize(movement_data.velocity), -1.0);

                    std::vector<RayCollision> hit_points;
                    if (!raycast(registry, transform_data.position, direction, hit_points, 75, false))
                    {
                        continue;
                    }
                    auto fartest_point = hit_points[0];

                    transform_data.position = Vector2{fartest_point.point.x, fartest_point.point.y};
                }
            }
        }
    }

   protected:
    entt::registry& registry;
};

#endif // BASE_PROC_HPP
