#ifndef COLLISION_DEF_HPP
#define COLLISION_DEF_HPP

#include <vector>

#include "base_definitions.hpp"
#include "raylib.h"

struct collider
{
    bool is_trigger;
};

struct rect_collider : collider
{
    Vector2 size;

    rect_collider(bool is_trigger, Vector2 extents) :
        collider{is_trigger}, size{extents} {}

    void generate_conners(std::vector<Vector2>& corners)
    {
        corners[0] = Vector2{-size.x / 2, -size.y / 2};
        corners[1] = Vector2{size.x / 2, -size.y / 2};
        corners[2] = Vector2{size.x / 2, size.y / 2};
        corners[3] = Vector2{-size.x / 2, size.y / 2};
    }
};

static RayCollision raycast_single_rect(transform& collider_transform, rect_collider& collider_data,
                                        Vector2& origin, Vector2& direction)
{
    // direction   = Vector2Normalize(direction);
    float angle = atan2(collider_transform.direction.y, collider_transform.direction.x);

    Matrix rotation_matrix = MatrixRotateZ(angle);
    Matrix translation_matrix =
        MatrixTranslate(collider_transform.position.x, collider_transform.position.y, 0);

    Matrix transform_matrix =
        MatrixMultiply(rotation_matrix, translation_matrix);

    //? Should this BB be generated once and stored?
    BoundingBox box = {
        Vector3{0 - collider_data.size.x / 2, 0 - collider_data.size.y / 2, -1},
        Vector3{0 + collider_data.size.x / 2, 0 + collider_data.size.y / 2, 1}};

    //! Relative operations in the rect space
    Matrix inverse_rotation_matrix = MatrixInvert(rotation_matrix);
    Vector2 relative_origin        = Vector2Subtract(origin, collider_transform.position);
    Vector3 ray_origin =
        Vector3Transform(Vector3{relative_origin.x, relative_origin.y, 0},
                         inverse_rotation_matrix);

    Vector3 ray_direction = Vector3Transform(
        Vector3{direction.x, direction.y, 0}, inverse_rotation_matrix);

    Ray ray          = {ray_origin, ray_direction};
    RayCollision hit = GetRayCollisionBox(ray, box);

    auto hit_point  = Vector3Transform(hit.point, transform_matrix);
    auto hit_normal = Vector3Transform(hit.normal, rotation_matrix);
    return RayCollision{hit.hit, hit.distance, hit_point, hit_normal};
}

static bool raycast(entt::registry& registry, Vector2 origin, Vector2 direction,
                    std::vector<RayCollision>& hit_points, float distance = 500, bool sort_closest = true)
{
    auto rect_collider_view = registry.view<transform, rect_collider>();
    direction               = Vector2Normalize(direction);

    for (auto [entity, transform_data, collider_data] : rect_collider_view.each())
    {
        RayCollision hit_point = raycast_single_rect(transform_data, collider_data, origin, direction);
        if (hit_point.hit && hit_point.distance <= distance)
        {
            hit_points.push_back(hit_point);
        }
    }

    if (hit_points.size() <= 0)
    {
        return false;
    }

    std::sort(hit_points.begin(), hit_points.end(), [&sort_closest](RayCollision a, RayCollision b) {
        return sort_closest ? a.distance < b.distance : a.distance > b.distance;
    });

    return true;
}

#endif // COLLISION_DEF_HPP
