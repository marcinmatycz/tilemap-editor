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
        std::get<UI::Triangle>(arrow).color = SKYBLUE;

        unsigned &index = app_state.tilemap_index;
        if (inputs.left_mouse_button == MouseButtonState::PRESSED)
        {
            index++;
            if (index == app_state.tilemaps.size())
            {
                index = 0;
            }

            const int initial_scale = app_state.texture_grid.square_size_px / app_state.tile_size;
            const int tile_size = app_state.tile_size;
            const int margin = app_state.texture_grid_margin;

            app_state.texture_grid = {
                .x_square_count = app_state.tilemaps[index].texture.width / tile_size + 2 * margin,
                .y_square_count = app_state.tilemaps[index].texture.height / tile_size + 2 * margin,
                .square_size_px = tile_size * initial_scale};

            UI::Text &text = std::get<UI::Text>(ui["tilemap_filename"]);
            text.text = app_state.tilemaps[app_state.tilemap_index].texture_filename;

            app_state.selected_tile = std::nullopt;
        }
    }
    else
    {
        // TODO: add hovered color and not hovered color
        std::get<UI::Triangle>(arrow).color = BLUE;
    }
}

inline void arrow_left(const Inputs &inputs, std::map<std::string, UI::Item> &ui, AppState &app_state,
                       const bool is_hovered)
{
    UI::Item &arrow = ui["tile_bank_arrow_left"];
    if (is_hovered)
    {
        // TODO: better colors in yaml, add predefined which map to raylib or something
        std::get<UI::Triangle>(arrow).color = SKYBLUE;
        if (inputs.left_mouse_button == MouseButtonState::PRESSED)
        {
            unsigned &index = app_state.tilemap_index;
            if (index == 0)
            {
                index = app_state.tilemaps.size() - 1;
            }
            else
            {
                index--;
            }

            const int initial_scale = app_state.texture_grid.square_size_px / app_state.tile_size;
            const int tile_size = app_state.tile_size;
            const int margin = app_state.texture_grid_margin;

            app_state.texture_grid = {
                .x_square_count = app_state.tilemaps[index].texture.width / tile_size + 2 * margin,
                .y_square_count = app_state.tilemaps[index].texture.height / tile_size + 2 * margin,
                .square_size_px = tile_size * initial_scale};
            UI::Text &text = std::get<UI::Text>(ui["tilemap_filename"]);
            text.text = app_state.tilemaps[app_state.tilemap_index].texture_filename;

            app_state.selected_tile = std::nullopt;
        }
    }
    else
    {
        // TODO: add hovered color and not hovered color
        std::get<UI::Triangle>(arrow).color = BLUE;
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
*/

} // namespace callbacks
