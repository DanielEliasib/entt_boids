#include "boids.hpp"
#include<iostream>
#include<raylib.h>

int main()
{
	InitWindow(800, 600, "Hello World");
	
	entt::registry registry = entt::registry();

	entt::scheduler general_scheduler;
	general_scheduler.attach<boids::movement_process>(registry);

	entt::scheduler render_scheduler;
	render_scheduler.attach<boids::render_process>(registry);

	boids::create_n_boids(registry, 100, Vector2{400, 300}, 100);

	while (!WindowShouldClose())
	{
		general_scheduler.update(GetFrameTime()*1000);

		BeginDrawing();
		ClearBackground(RAYWHITE);
		DrawText("BOIDS!", 10, 10, 20, GOLD);

		render_scheduler.update(GetFrameTime()*1000);

		EndDrawing();
	}
	return 0;
}
