#include "base_definitions.hpp"
#include "raylib.h"
#include <entt.hpp>
#include <raymath.h>

struct collider {
    bool is_trigger;
};

struct rect_collider : collider {
    Vector2 extents;

    rect_collider(bool is_trigger, Vector2 extents)
        : collider{is_trigger}, extents{extents} {}
};

struct circle_collider : collider {
    float radius;
};

static void generate_rect_conners(Vector2 extents,
                                  std::vector<Vector2> &corners) {
    corners[0] = Vector2{-extents.x / 2, -extents.y / 2};
    corners[1] = Vector2{extents.x / 2, -extents.y / 2};
    corners[2] = Vector2{extents.x / 2, extents.y / 2};
    corners[3] = Vector2{-extents.x / 2, extents.y / 2};
}

static bool raycast(entt::registry &registry, Vector2 origin, Vector2 direction,
                    float distance = 500, Vector2 *collision_point = nullptr) {
    // rect collisions first
    auto rect_collider_view = registry.view<transform, rect_collider>();
    direction = Vector2Normalize(direction);

    for (auto [entity, transform, collider] : rect_collider_view.each()) {
        std::vector<Vector2> collisions;
        collisions.reserve(4);

        float angle = atan2(transform.direction.y, transform.direction.x);
        auto RotationMatrix = MatrixRotateZ(angle);
        auto TranslatedMatrix =
            MatrixTranslate(transform.position.x, transform.position.y, 0);
        auto FullMatrix = MatrixMultiply(TranslatedMatrix, RotationMatrix);
        auto InvertedFullMatrix = MatrixInvert(FullMatrix);

        BoundingBox box = {Vector3{0 - collider.extents.x / 2,
                                   0 - collider.extents.y / 2, -10},
                           Vector3{0 + collider.extents.x / 2,
                                   0 + collider.extents.y / 2, 10}};

        Ray ray = {Vector3Transform(Vector3{origin.x, origin.y, 0},
                                    MatrixInvert(TranslatedMatrix)),
                   Vector3Transform(Vector3{direction.x, direction.y, 0},
                                    MatrixInvert(RotationMatrix))};

        auto hit = GetRayCollisionBox(ray, box);

        if (hit.hit) {
            auto hit_point =
                Vector2Transform(Vector2{hit.point.x, hit.point.y}, FullMatrix);
            if (collision_point != nullptr) {
                *collision_point = hit_point;
            }
            return true;
        }
    }
    return false;
}
