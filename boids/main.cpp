#include <base_definitions.hpp>
#include <base_processors.hpp>
#include <boids.hpp>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

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

void create_screen_walls(entt::registry &registry) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    int horizontal_lenght = width;
    int vertical_lenght = height;

    auto top_wall = registry.create();
    auto bottom_wall = registry.create();
    auto left_wall = registry.create();
    auto right_wall = registry.create();

    std::vector<Vector2> horizontal_corners;
    horizontal_corners.resize(4);

    std::vector<Vector2> vertical_corners;
    vertical_corners.resize(4);

    rect_collider horizontal_collider(
        false, Vector2{static_cast<float>(horizontal_lenght), 5});
    rect_collider vertical_collider(
        false, Vector2{5, static_cast<float>(vertical_lenght)});

    horizontal_collider.generate_conners(horizontal_corners);
    vertical_collider.generate_conners(vertical_corners);

    registry.emplace<transform>(
        top_wall, transform{Vector2{width / 2.0f, 0}, Vector2{1, 0}});
    registry.emplace<rect_collider>(top_wall, horizontal_collider);
    registry.emplace<renderable>(top_wall,
                                 renderable{BLUE, 1, horizontal_corners});

    registry.emplace<transform>(
        bottom_wall,
        transform{Vector2{width / 2.0f, static_cast<float>(height)},
                  Vector2{1, 0}});
    registry.emplace<rect_collider>(bottom_wall, horizontal_collider);
    registry.emplace<renderable>(bottom_wall,
                                 renderable{BLUE, 1, horizontal_corners});

    registry.emplace<transform>(
        left_wall, transform{Vector2{0, height / 2.0f}, Vector2{1, 0}});
    registry.emplace<rect_collider>(left_wall, vertical_collider);
    registry.emplace<renderable>(left_wall,
                                 renderable{BLUE, 1, vertical_corners});

    registry.emplace<transform>(
        right_wall, transform{Vector2{static_cast<float>(width), height / 2.0f},
                              Vector2{1, 0}});
    registry.emplace<rect_collider>(right_wall, vertical_collider);
    registry.emplace<renderable>(right_wall,
                                 renderable{BLUE, 1, vertical_corners});
}

void random_block(entt::registry& registry)
{
	int width = GetScreenWidth();
	int height = GetScreenHeight();

	int horizontal_lenght = GetRandomValue(50, 200);
	int vertical_lenght = GetRandomValue(50, 200);

	std::vector<Vector2> corners;
	corners.resize(4);

	rect_collider collider(false, Vector2{ static_cast<float>(horizontal_lenght), static_cast<float>(vertical_lenght) });
	collider.generate_conners(corners);

	auto block = registry.create();
	registry.emplace<transform>(block, transform{ Vector2{static_cast<float>(GetRandomValue(0, width)), static_cast<float>(GetRandomValue(0, height))}, Vector2{1, 0} });
	registry.emplace<rect_collider>(block, collider);
	registry.emplace<renderable>(block, renderable{ BLUE, 1, corners});
}

int main() {
    InitWindow(800, 600, "BOIDS");

    entt::registry registry = entt::registry();

    entt::scheduler general_scheduler;
    general_scheduler.attach<movement_process>(registry);

    entt::scheduler render_scheduler;
    render_scheduler.attach<render_process>(registry);
    render_scheduler.attach<vision_process>(registry);

    boids::create_n_boids(registry, 100, Vector2{400, 300}, 100);

    create_screen_walls(registry);
	random_block(registry);

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

        auto direction = Vector2Scale(
            Vector2Normalize(Vector2Subtract(random_screen_position_2,
                                             random_screen_position_1)),
            500);
        DrawLineEx(random_screen_position_1,
                   Vector2Add(random_screen_position_1, direction), 1.0f,
                   intercepted ? RED : GREEN);

        DrawCircleV(random_screen_position_1, 5, BLUE);
        DrawCircleV(random_screen_position_2, 5, YELLOW);

        EndDrawing();
    }
    return 0;
}
