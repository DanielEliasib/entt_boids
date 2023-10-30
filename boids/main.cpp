#include <base_definitions.hpp>
#include <base_processors.hpp>
#include <boids.hpp>
#include <raymath.h>
#include <collision_definitions.hpp>
#include <iostream>
#include <raylib.h>

bool random_raycast(entt::registry &registry, Vector2 *collision_point,
                    Vector2 *random_screen_position_1,
                    Vector2 *random_screen_position_2) {
    *random_screen_position_1 =
        Vector2{(float)GetRandomValue(0, 800), (float)GetRandomValue(0, 600)};
    *random_screen_position_2 =
        Vector2{(float)GetRandomValue(0, 800), (float)GetRandomValue(0, 600)};

    return raycast(registry, *random_screen_position_1,
                   Vector2Normalize(Vector2Subtract(*random_screen_position_2,
                                                    *random_screen_position_1)),
                   500, collision_point);
}

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

	Vector2 rect_direction = Vector2Normalize(Vector2{static_cast<float>(GetRandomValue(-100, 100)), static_cast<float>(GetRandomValue(-100, 100))});
    registry.emplace<transform>(collision_entity,
                                transform{Vector2{400, 300}, rect_direction});

    registry.emplace<rect_collider>(collision_entity,
                                    rect_collider(false, extents));
    registry.emplace<renderable>(collision_entity,
                                 renderable(BLUE, 10, corners));

    Vector2 random_screen_position_1;
    Vector2 random_screen_position_2;

    Vector2 collision_point;
    bool intercepted =
        random_raycast(registry, &collision_point, &random_screen_position_1,
                       &random_screen_position_2);

    while (!WindowShouldClose()) {
        general_scheduler.update(GetFrameTime() * 1000);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("BOIDS!", 10, 10, 20, GOLD);

        render_scheduler.update(GetFrameTime() * 1000);

        if (IsKeyPressed(KEY_SPACE)) {
            intercepted = random_raycast(registry, &collision_point,
                                         &random_screen_position_1,
                                         &random_screen_position_2);
        }

        if (intercepted) {
            DrawCircleV(collision_point, 20, RED);
        }

		auto direction = Vector2Scale(Vector2Normalize(Vector2Subtract(random_screen_position_2,
														 random_screen_position_1)), 500);
        DrawLineEx(random_screen_position_1, Vector2Add(random_screen_position_1,direction), 1.0f,
                   intercepted ? RED : GREEN);

		DrawCircleV(random_screen_position_1, 5, BLUE);
		DrawCircleV(random_screen_position_2, 5, YELLOW);

        EndDrawing();
    }
    return 0;
}
