#ifndef COLLISION_DEF_HPP
#define COLLISION_DEF_HPP

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

    void generate_conners(std::vector<Vector2> &corners) {
        corners[0] = Vector2{-extents.x / 2, -extents.y / 2};
        corners[1] = Vector2{extents.x / 2, -extents.y / 2};
        corners[2] = Vector2{extents.x / 2, extents.y / 2};
        corners[3] = Vector2{-extents.x / 2, extents.y / 2};
    }
};

struct circle_collider : collider {
    float radius;
};

static bool raycast(entt::registry &registry, Vector2 origin, Vector2 direction,
                    float distance = 500, RayCollision *collision_point = nullptr) {
    // rect collisions first
    auto rect_collider_view = registry.view<transform, rect_collider>();
    direction = Vector2Normalize(direction);

	//? should we reserve some space?
	std::vector<RayCollision> hit_points;

    for (auto [entity, transform, collider] : rect_collider_view.each()) {
        float angle = atan2(transform.direction.y, transform.direction.x);
        Matrix rotation_matrix = MatrixRotateZ(angle);

        Matrix translation_matrix =
            MatrixTranslate(transform.position.x, transform.position.y, 0);
        Matrix transform_matrix =
            MatrixMultiply(rotation_matrix, translation_matrix);

        //? Should this BB be generated once and stored?
        BoundingBox box = {
            Vector3{0 - collider.extents.x / 2, 0 - collider.extents.y / 2, -1},
            Vector3{0 + collider.extents.x / 2, 0 + collider.extents.y / 2, 1}};

        //! Relative operations in the rect space
        Matrix inverse_rotation_matrix = MatrixInvert(rotation_matrix);
        Vector2 relative_origin = Vector2Subtract(origin, transform.position);
        Vector3 ray_origin =
            Vector3Transform(Vector3{relative_origin.x, relative_origin.y, 0},
                             inverse_rotation_matrix);

        Vector3 ray_direction = Vector3Transform(
            Vector3{direction.x, direction.y, 0}, inverse_rotation_matrix);

        Ray ray = {ray_origin, ray_direction};
        auto hit = GetRayCollisionBox(ray, box);

        if (hit.hit && hit.distance <= distance) {
            auto hit_point = Vector3Transform(hit.point, transform_matrix);
			auto hit_normal = Vector3Transform(hit.normal, rotation_matrix);
			hit_points.push_back(RayCollision{hit.hit, hit.distance, hit_point, hit_normal});
        }
    }

	if(hit_points.size() <= 0)
	{
		return false;
	}

	std::sort(hit_points.begin(), hit_points.end(), [&origin](RayCollision a, RayCollision b) {
		return a.distance < b.distance;
	});

	if(collision_point != nullptr)
	{
		*collision_point = hit_points[0];
	}

    return true;
}

#endif // COLLISION_DEF_HPP
