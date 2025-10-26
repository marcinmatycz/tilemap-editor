#pragma once
#include <cassert>
#include <string>
#include "raylib.h"

struct Tilemap
{
    std::string texture_filename;
    Texture2D texture;
};

enum class MouseButtonState
{
    UP,
    DOWN,
    PRESSED,
    RELEASED,
};

struct Inputs
{
    Vector2 mouse_point{};
    MouseButtonState left_mouse_button{};
    MouseButtonState right_mouse_button{};
    float wheel{};
};

struct Grid
{
    int x_square_count{};
    int y_square_count{};
    int square_size_px{};
};

struct AppState
{
    Grid main_grid{};
    Grid texture_grid{};
    Camera2D main_camera{};
    Camera2D texture_camera{};
    unsigned tilemap_index{0};
};

inline MouseButtonState get_mouse_button_state(const MouseButton button)
{
    if (IsMouseButtonReleased(button))
    {
        return MouseButtonState::RELEASED;
    }
    else if (IsMouseButtonUp(button))
    {
        return MouseButtonState::UP;
    }
    else if (IsMouseButtonPressed(button))
    {
        return MouseButtonState::PRESSED;
    }
    else if (IsMouseButtonDown(button))
    {
        return MouseButtonState::DOWN;
    }
    assert(false);
    return MouseButtonState::UP;
}

inline Inputs get_inputs()
{
    return {GetMousePosition(), get_mouse_button_state(MOUSE_BUTTON_LEFT), get_mouse_button_state(MOUSE_BUTTON_RIGHT),
            GetMouseWheelMove()};
}
