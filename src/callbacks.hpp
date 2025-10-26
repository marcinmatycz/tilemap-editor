#pragma once
#include <map>
#include <array>
#include "engine_core.hpp"
#include "ui.hpp"
#include "raylib.h"
#include "raymath.h"

namespace callbacks
{

inline void reload_button(const Inputs &, std::map<std::string, UI::Item> &ui, AppState &, const bool is_hovered)
{
    UI::Item &button = ui["reload_button"];
    if (is_hovered)
    {
        // TODO: better colors in yaml, add predefined which map to raylib or something
        std::get<UI::Textbox>(button).box.color = RED;
    }
    else
    {
        // TODO: add hovered color and not hovered color
        std::get<UI::Textbox>(button).box.color = BLUE;
    }
}

inline void arrow_right(const Inputs &inputs, std::map<std::string, UI::Item> &ui, AppState &app_state,
                        const bool is_hovered)
{
    UI::Item &arrow = ui["tile_bank_arrow_right"];
    if (is_hovered)
    {
        // TODO: better colors in yaml, add predefined which map to raylib or something
        std::get<UI::Triangle>(arrow).color.a += 40;
        if (inputs.left_mouse_button == MouseButtonState::PRESSED)
        {
            app_state.tilemap_index++;
            if (app_state.tilemap_index == app_state.tilemaps.size())
            {
                app_state.tilemap_index = 0;
            }
            UI::Text &text = std::get<UI::Text>(ui["tilemap_filename"]);
            text.text = app_state.tilemaps[app_state.tilemap_index].texture_filename;
        }
    }
    else
    {
        // TODO: add hovered color and not hovered color
        std::get<UI::Triangle>(arrow).color.a -= 40;
    }
}

inline void arrow_left(const Inputs &inputs, std::map<std::string, UI::Item> &ui, AppState &app_state,
                       const bool is_hovered)
{
    UI::Item &arrow = ui["tile_bank_arrow_left"];
    if (is_hovered)
    {
        // TODO: better colors in yaml, add predefined which map to raylib or something
        std::get<UI::Triangle>(arrow).color.a += 40;
        if (inputs.left_mouse_button == MouseButtonState::PRESSED)
        {
            if (app_state.tilemap_index == 0)
            {
                app_state.tilemap_index = app_state.tilemaps.size() - 1;
            }
            else
            {
                app_state.tilemap_index--;
            }
            UI::Text &text = std::get<UI::Text>(ui["tilemap_filename"]);
            text.text = app_state.tilemaps[app_state.tilemap_index].texture_filename;
        }
    }
    else
    {
        // TODO: add hovered color and not hovered color
        std::get<UI::Triangle>(arrow).color.a -= 40;
    }
}

// TODO: move somewhere else
inline std::array<Vector2, 2> get_camera_boundaries(const Grid &grid)
{
    const Vector2 min{-grid.square_size_px, -grid.square_size_px};
    const Vector2 max{(grid.x_square_count + 1) * grid.square_size_px, (grid.y_square_count + 1) * grid.square_size_px};
    return {min, max};
}

inline void pan_camera(Camera2D &camera, const Vector2 &clamp_min, const Vector2 &clamp_max)
{
    const Vector2 delta = Vector2Scale(GetMouseDelta(), -1.0f / camera.zoom);
    camera.target = Vector2Clamp(Vector2Add(camera.target, delta), clamp_min, clamp_max);
}

inline void zoom_camera(Camera2D &camera, const Inputs &inputs)
{
    const float scale = 0.2f * inputs.wheel;
    const Vector2 mouseWorldPos = GetScreenToWorld2D(inputs.mouse_point, camera);
    camera.offset = inputs.mouse_point;
    camera.target = mouseWorldPos;
    camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
}

inline void main_area(const Inputs &inputs, std::map<std::string, UI::Item> &, AppState &app_state,
                      const bool is_hovered)
{
    if (is_hovered)
    {
        if ((inputs.right_mouse_button == MouseButtonState::DOWN) or
            (inputs.right_mouse_button == MouseButtonState::PRESSED))
        {
            const auto [min, max] = get_camera_boundaries(app_state.main_grid);
            pan_camera(app_state.main_camera, min, max);
        }
        if (inputs.wheel != 0)
        {
            zoom_camera(app_state.main_camera, inputs);
        }
    }
}

inline void texture_area(const Inputs &inputs, std::map<std::string, UI::Item> &, AppState &app_state,
                         const bool is_hovered)
{
    if (is_hovered)
    {
        if ((inputs.right_mouse_button == MouseButtonState::DOWN) or
            (inputs.right_mouse_button == MouseButtonState::PRESSED))
        {
            const auto [min, max] = get_camera_boundaries(app_state.texture_grid);
            pan_camera(app_state.texture_camera, min, max);
        }
        if (inputs.wheel != 0)
        {
            zoom_camera(app_state.texture_camera, inputs);
        }
    }
}

/*
if (CheckCollisionPointRec(inputs.mouse_point, reload_button.rectangle))
{
    reload_button.color.a += 40;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        config = YAML::LoadFile(config_path);
        interface = load_interface(config);
        tilebank_array = load_tilebank_array(config);
    }
}
else if (CheckCollisionPointTriangle(inputs.mouse_point, left_arrow.p1, left_arrow.p2, left_arrow.p3))
{
    left_arrow.color.a += 40;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (tilemap_index == 0)
        {
            tilemap_index = tilemaps.size() - 1;
        }
        else
        {
            tilemap_index--;
        }

        UI::Text &text = std::get<UI::Text>(interface["tilemap_filename"]);
        text.text = tilemaps[tilemap_index].texture_filename;
    }
}
else if (CheckCollisionPointTriangle(inputs.mouse_point, right_arrow.p1, right_arrow.p2, right_arrow.p3))
{
    right_arrow.color.a += 40;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        tilemap_index++;
        if (tilemap_index == tilemaps.size())
        {
            tilemap_index = 0;
        }
        UI::Text &text = std::get<UI::Text>(interface["tilemap_filename"]);
        text.text = tilemaps[tilemap_index].texture_filename;
    }
}
*/

} // namespace callbacks
