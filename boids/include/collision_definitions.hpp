#include "base_definitions.hpp"
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

static void generate_rect_conners(Vector2 extents, std::vector<Vector2>& corners) {
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

        float angle = atan2(direction.y, direction.x);
        auto RotationMatrix = MatrixRotateZ(angle);

        // v1----v4
        // |      |
        // v2----v3
        Vector2 rect_v1 =
            Vector2Add(transform.position,
                       Vector2Transform(Vector2{-collider.extents.x / 2,
                                                -collider.extents.y / 2},
                                        RotationMatrix));
        Vector2 rect_v2 =
            Vector2Add(transform.position,
                       Vector2Transform(Vector2{collider.extents.x / 2,
                                                -collider.extents.y / 2},
                                        RotationMatrix));
        Vector2 rect_v3 =
            Vector2Add(transform.position,
                       Vector2Transform(Vector2{collider.extents.x / 2,
                                                collider.extents.y / 2},
                                        RotationMatrix));
        Vector2 rect_v4 =
            Vector2Add(transform.position,
                       Vector2Transform(Vector2{-collider.extents.x / 2,
                                                collider.extents.y / 2},
                                        RotationMatrix));

        bool collides_1 = CheckCollisionLines(
            origin, Vector2Add(origin, Vector2Scale(direction, distance)),
            rect_v1, rect_v2, &collisions[0]);
        bool collides_2 = CheckCollisionLines(
            origin, Vector2Add(origin, Vector2Scale(direction, distance)),
            rect_v2, rect_v3, &collisions[1]);
        bool collides_3 = CheckCollisionLines(
            origin, Vector2Add(origin, Vector2Scale(direction, distance)),
            rect_v3, rect_v4, &collisions[2]);
        bool collides_4 = CheckCollisionLines(
            origin, Vector2Add(origin, Vector2Scale(direction, distance)),
            rect_v4, rect_v1, &collisions[3]);

        if (collides_1 || collides_2 || collides_3 || collides_4) {
            printf("Collision with rect collider\n");
            std::sort(collisions.begin(), collisions.end(),
                      [origin](Vector2 a, Vector2 b) {
                          return Vector2DistanceSqr(origin, a) <
                                 Vector2DistanceSqr(origin, b);
                      });

            *collision_point = collisions[0];
            return true;
        }
    }
	return false;
}
