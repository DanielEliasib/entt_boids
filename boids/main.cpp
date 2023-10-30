#include <base_definitions.hpp>
#include <base_processors.hpp>
#include <boids.hpp>
#include <collision_definitions.hpp>
#include <iostream>
#include <raylib.h>

int main() {
    InitWindow(800, 600, "Hello World");

    entt::registry registry = entt::registry();

    entt::scheduler general_scheduler;
    general_scheduler.attach<movement_process>(registry);

    entt::scheduler render_scheduler;
    render_scheduler.attach<render_process>(registry);

    boids::create_n_boids(registry, 100, Vector2{400, 300}, 100);

    Vector2 extents{150, 100};
    std::vector<Vector2> corners;
    corners.resize(4);

    std::cout << "Corner size: " << corners.size() << "\n";

    generate_rect_conners(extents, corners);

    std::cout << "Corner size: " << corners.size() << "\n";

    auto collision_entity = registry.create();
    registry.emplace<transform>(collision_entity,
                                transform{Vector2{400, 300}, Vector2{1, 0}});
    registry.emplace<rect_collider>(collision_entity,
                                    rect_collider(false, extents));
    registry.emplace<renderable>(collision_entity,
                                 renderable(BLUE, 10, corners));

    Vector2 random_screen_position_1 =
        Vector2{(float)GetRandomValue(0, 800), (float)GetRandomValue(0, 600)};
    Vector2 random_screen_position_2 =
        Vector2{(float)GetRandomValue(0, 800), (float)GetRandomValue(0, 600)};

    Vector2 collision_point;
    bool intercepted =
        raycast(registry, random_screen_position_1,
                Vector2Normalize(Vector2Subtract(random_screen_position_2,
                                                 random_screen_position_1)),
                500, &collision_point);


    while (!WindowShouldClose()) {
        general_scheduler.update(GetFrameTime() * 1000);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("BOIDS!", 10, 10, 20, GOLD);

        render_scheduler.update(GetFrameTime() * 1000);

		if (intercepted) {
			DrawCircleV(collision_point, 20, RED);
		}
		DrawLineEx(random_screen_position_1, random_screen_position_2, 1.0f, intercepted ? RED : GREEN);

        EndDrawing();
    }
    return 0;
}
