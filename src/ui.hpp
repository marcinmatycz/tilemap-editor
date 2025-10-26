#pragma once
#include <string>
#include <variant>
#include "engine_core.hpp"
#include "raylib.h"

namespace UI
{

struct Box
{
    Rectangle rectangle;
    Color color;
};

struct Triangle
{
    Vector2 p1;
    Vector2 p2;
    Vector2 p3;
    Color color;
};

struct Text
{
    int x;
    int y;
    int size;
    std::string text;
    Color color;
};

struct Textbox
{
    Box box;
    Text text;
};

using Item = std::variant<Box, Triangle, Text, Textbox>;

inline bool is_hovered(const Box &box, const Inputs &inputs)
{
    return CheckCollisionPointRec(inputs.mouse_point, box.rectangle);
}

inline bool is_hovered(const Triangle &triangle, const Inputs &inputs)
{
    return CheckCollisionPointTriangle(inputs.mouse_point, triangle.p1, triangle.p2, triangle.p3);
}

inline bool is_hovered(const Text &, const Inputs &) { return false; }

inline bool is_hovered(const Textbox &textbox, const Inputs &inputs)
{
    return CheckCollisionPointRec(inputs.mouse_point, textbox.box.rectangle);
}

inline bool is_hovered(const Item &item, const Inputs &inputs)
{
    const auto visitor = [&inputs](const auto &i) { return is_hovered(i, inputs); };

    return item.visit(visitor);
}

} // namespace UI
