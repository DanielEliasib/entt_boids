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
        float angle = atan2(transform.direction.y, transform.direction.x);

        auto RotationMatrix = MatrixRotateZ(angle);
        Vector2 rotated_identity =
            Vector2Transform(Vector2{1, 0}, RotationMatrix);
        auto TranslatedMatrix =
            MatrixTranslate(transform.position.x, transform.position.y, 0);
        auto FullMatrix = MatrixMultiply(TranslatedMatrix, RotationMatrix);

        BoundingBox box = {Vector3{0 - collider.extents.x / 2,
                                   0 - collider.extents.y / 2, -10},
                           Vector3{0 + collider.extents.x / 2,
                                   0 + collider.extents.y / 2, 10}};

		Vector2 relative_origin = Vector2Subtract(origin, transform.position);
		Vector3 ray_origin = Vector3Transform(Vector3{relative_origin.x, relative_origin.y, 0}, MatrixInvert(RotationMatrix));
		Vector3 ray_direction = Vector3Transform(Vector3{direction.x, direction.y, 0}, MatrixInvert(RotationMatrix));

        Ray ray = {ray_origin, ray_direction};
        auto hit = GetRayCollisionBox(ray, box);

        if (hit.hit) {
            auto hit_point =
                Vector3Transform(Vector3{hit.point.x, hit.point.y, 0}, RotationMatrix);
			hit_point = Vector3Transform(hit_point, TranslatedMatrix);
            if (collision_point != nullptr) {
                *collision_point = Vector2{hit_point.x, hit_point.y};
            }
            return true;
        }
    }
    return false;
}
