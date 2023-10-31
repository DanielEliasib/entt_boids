#ifndef BOIDS_DEF_HPP
#define BOIDS_DEF_HPP

#include "collision_definitions.hpp"
#include "base_definitions.hpp"
#include <entt.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

namespace boids {

struct collision_avoidance_process
    : entt::process<collision_avoidance_process, std::uint32_t> {
    using delta_type = std::uint32_t;

    collision_avoidance_process(entt::registry &registry)
        : registry(registry) {}

    void update(delta_type delta_time, void *) {
        auto boids_view = registry.view<transform, movement>();
        for (auto [entity, transform_data, movement_data] : boids_view.each()) {
            Vector2 collision_point;
            auto hit = raycast(registry, transform_data.position,
                               transform_data.direction, 30, &collision_point);

            if (hit) {
                movement_data.velocity = Vector2Reflect(
                    movement_data.velocity,
                    Vector2Normalize(Vector2Subtract(collision_point,
                                                     transform_data.position)));
            }
        }
    }

  protected:
    entt::registry &registry;
};

} // namespace boids

#endif // BOIDS_DEF_HPP
