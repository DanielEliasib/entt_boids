#pragma once
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <entt.hpp>

struct transform {
    Vector2 position;
    Vector2 direction;
};

struct movement {
    Vector2 velocity;
};

struct renderable {
    Color color;
    float size;

    std::vector<Vector2> vertices;

    renderable(Color color, float size, std::vector<Vector2> &vertices)
        : color(color), size(size) {
        this->vertices = vertices;
    }
};


