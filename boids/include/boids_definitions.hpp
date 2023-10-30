#pragma once

#include <cstdint>
#include <entt.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

namespace boids {



struct collision_process : entt::process<collision_process, std::uint32_t>
{
	using delta_type = std::uint32_t;

	collision_process(entt::registry &registry) : registry(registry) {}

	

	protected:
	entt::registry &registry;
};


} // namespace boids
