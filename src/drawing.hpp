#pragma once
#include <vector>
#include <variant>
#include <map>
#include <optional>
#include "raylib.h"
#include "engine_core.hpp"
#include "ui.hpp"
#include "yaml-cpp/yaml.h"


namespace drawing
{

inline void draw_main_area(const AppState &app_state)
{
    for (int i = 0; i < app_state.main_grid.x_square_count; i++)
    {
	for (int j = 0; j < app_state.main_grid.y_square_count; j++)
	{
	    const int square_size = app_state.main_grid.square_size_px;
	    DrawRectangleLines(i * square_size, j * square_size, square_size, square_size, BLACK);
	}
    }
    //TODO: remove
    DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 50, MAROON);
}

inline void draw_texture_area(const AppState &app_state, const Texture2D &texture, const YAML::Node &config)
{
    std::optional<Rectangle> highlighted{std::nullopt};
    const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), app_state.texture_camera);

    for (int i = 0; i < app_state.texture_grid.x_square_count; i++)
    {
	for (int j = 0; j < app_state.texture_grid.y_square_count; j++)
	{
	    const int square_size = app_state.texture_grid.square_size_px;
	    if (CheckCollisionPointRec(mouseWorldPos, Rectangle{.x = i * square_size,
			.y = j * square_size,
			.width = square_size,
			.height = square_size}))
	    {
		highlighted = {
		    .x = i * square_size, .y = j * square_size, .width = square_size, .height = square_size};
	    }
	    DrawRectangleLines(i * square_size, j * square_size, square_size, square_size, BLACK);
	}
    }

    const int tile_size = config["tile_size_px"].as<int>();
    const int scale = config["texture_grid"]["initial_scale"].as<int>();
    const int margin = config["texture_grid"]["margin"].as<int>();

    const float texture_offset = tile_size * scale * margin;
    DrawTextureEx(texture, {texture_offset, texture_offset}, 0, scale, WHITE);
    if (highlighted)
    {
	Color highlight = BLUE;
	highlight.a = 100;
	DrawRectangleRec(highlighted.value(), highlight);
    }
}

inline void draw_ui( const std::vector<std::vector<std::string>> &layers, std::map<std::string, UI::Item> &ui) 
{
    for(auto layer = layers.rbegin(); layer != layers.rend(); layer++)
    {
	for(const std::string &key: *layer)
	{
	    const auto &item = ui[key];
	    //TODO: add visitors with drawing to ui.hpp
	    if (std::holds_alternative<UI::Box>(item))
	    {
		const auto &box = std::get<UI::Box>(item);
		DrawRectangleRec(box.rectangle, box.color);
	    }
	    else if (std::holds_alternative<UI::Textbox>(item))
	    {
		const auto &box = std::get<UI::Textbox>(item).box;
		DrawRectangleRec(box.rectangle, box.color);

		const auto &text = std::get<UI::Textbox>(item).text;
		DrawText(text.text.c_str(), text.x, text.y, text.size, text.color);
	    }
	    else if (std::holds_alternative<UI::Triangle>(item))
	    {
		const auto &triangle = std::get<UI::Triangle>(item);
		DrawTriangle(triangle.p1, triangle.p2, triangle.p3, triangle.color);
	    }
	    else if (std::holds_alternative<UI::Text>(item))
	    {
		const auto &text = std::get<UI::Text>(item);
		DrawText(text.text.c_str(), text.x, text.y, text.size, text.color);
	    }

	}
    }
}

}
