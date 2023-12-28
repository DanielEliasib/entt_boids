#include <raylib.h>
#include <raymath.h>

#include <base_definitions.hpp>
#include <base_processors.hpp>
#include <boids.hpp>
#include <iostream>
#include <vector>

#include "boids_definitions.hpp"

bool random_raycast(entt::registry& registry, RayCollision* collision_point,
                    Vector2* random_screen_position_1,
                    Vector2* random_screen_position_2)
{
    *random_screen_position_1 =
        Vector2{(float)GetRandomValue(0, 800), (float)GetRandomValue(0, 600)};

    *random_screen_position_2 =
        Vector2{(float)GetRandomValue(0, 800), (float)GetRandomValue(0, 600)};

    std::vector<RayCollision> hit_points;
    auto hit = raycast(registry, *random_screen_position_1,
                       Vector2Normalize(Vector2Subtract(*random_screen_position_2, *random_screen_position_1)),
                       hit_points, 500);

    if (collision_point != nullptr)
    {
        collision_point = &hit_points[0];
    }

    return hit;
}

void create_screen_walls(entt::registry& registry)
{
    int width  = GetScreenWidth();
    int height = GetScreenHeight();

    int horizontal_lenght = width * 2;
    int vertical_lenght   = height * 2;

    auto top_wall    = registry.create();
    auto bottom_wall = registry.create();
    auto left_wall   = registry.create();
    auto right_wall  = registry.create();

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
    int width  = GetScreenWidth();
    int height = GetScreenHeight();

    int horizontal_lenght = GetRandomValue(50, 100);
    int vertical_lenght   = GetRandomValue(50, 100);

    std::vector<Vector2> corners;
    corners.resize(4);

    rect_collider collider(false, Vector2{static_cast<float>(horizontal_lenght),
                                          static_cast<float>(vertical_lenght)});
    collider.generate_conners(corners);

    auto block = registry.create();
    registry.emplace<transform>(
        block,
        transform{Vector2{static_cast<float>(GetRandomValue(0, width)),
                          static_cast<float>(GetRandomValue(0, height))},
                  Vector2Normalize({GetRandomValue(-100, 100) / 100.0f,
                                    GetRandomValue(-100, 100) / 100.0f})});
    registry.emplace<rect_collider>(block, collider);
    registry.emplace<renderable>(block, renderable{BLUE, 1, corners});
}

int main()
{
    InitWindow(800, 600, "BOIDS");
    SetRandomSeed(100);

    entt::registry registry = entt::registry();

    boids::create_n_boids(registry, 400, Vector2{400, 300}, 330);

    entt::scheduler general_scheduler;
    general_scheduler.attach<boids::boid_hashing_process>(registry);
    general_scheduler.attach<boids::cell_data_process>(registry);
    general_scheduler.attach<boids::boid_algo_process>(registry);
    general_scheduler.attach<movement_process>(registry);
    general_scheduler.attach<boids_constraints_process>(registry);

    entt::scheduler render_scheduler;
    render_scheduler.attach<render_process>(registry);
    // render_scheduler.attach<vision_process>(registry);
    render_scheduler.attach<boids::cell_renderer_process>(registry);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("BOIDS!", 10, 10, 20, GOLD);

        // auto delta_time = GetFrameTime() * 1000;
        float delta_time = 27;
        general_scheduler.update(delta_time);

        render_scheduler.update(delta_time);
        EndDrawing();
    }
    return 0;
}
