#include <array>
#include <cassert>
#include <map>
#include <optional>
#include <string>
#include "raylib.h"
#include "rlgl.h"
#include "yaml-cpp/yaml.h"
#include "config.hpp"
#include "engine_core.hpp"
#include "ui.hpp"
#include "callbacks.hpp"
#include "drawing.hpp"


/*
struct Tilemap
{
    std::string texture_filename;
    Texture2D texture;
    std::vector<Rectangle> tiles;
};
*/

using Callback = void(*)(const Inputs& inputs, std::map<std::string, UI::Item> &ui, AppState &app_state, bool is_hovered);

std::optional<std::string> get_ui_interaction(const Inputs &inputs, const std::vector<std::vector<std::string>> &layers, std::map<std::string, UI::Item> &ui)
{
    for(const std::vector<std::string> &layer: layers)
    {
	for(const std::string &item: layer)
	{
	    if(UI::is_hovered(ui[item], inputs))
	    {
		return item;
	    }

	}
    }

    return std::nullopt;
};


/*
std::vector<Rectangle> load_tilebank_array(const YAML::Node &config)
{
    const float tile_size = config["tile_size"].as<float>();
    const float position_x = config["tile_bank"]["position_x"].as<float>();
    const float position_y = config["tile_bank"]["position_y"].as<float>();
    const int tilebank_array_x = config["tile_bank"]["count_x"].as<int>();
    const int tilebank_array_y = config["tile_bank"]["count_y"].as<int>();
    const float gap = config["tile_bank"]["gap"].as<float>();
    const float scale = config["tile_bank"]["scale"].as<float>();

    std::vector<Rectangle> tilebank_array;
    tilebank_array.reserve(static_cast<unsigned>(tilebank_array_x * tilebank_array_y));

    const float offset_x = position_x * GetScreenWidth();
    const float offset_y = position_y * GetScreenHeight();

    for (int i = 0; i < (tilebank_array_x * tilebank_array_y); i++)
    {

        tilebank_array.push_back({.x = offset_x + i % tilebank_array_x * ((scale * tile_size) + gap),
                                  .y = offset_y + i / tilebank_array_x * ((scale * tile_size) + gap),
                                  .width = scale * tile_size,
                                  .height = scale * tile_size});
    }
    return tilebank_array;
}
*/

int main(void)
{
    const std::string config_path = "../resources/config.yaml";

    YAML::Node config = YAML::LoadFile(config_path);

    InitWindow(0, 0, config["window_name"].as<std::string>().c_str());
    while (not IsWindowReady())
    {
    };
    if (config["screen"]["fullscreen"].as<bool>())
    {
	ToggleBorderlessWindowed();
    }
    const auto [screen_width, screen_height] = config::get_screen_size(config);
    SetWindowSize(screen_width, screen_height);

    auto [layers, ui] = config::load_interface(config, screen_width, screen_height);
    std::map<std::string, Callback> ui_callbacks;
    ui_callbacks["reload_button"] = callbacks::reload_button;
    ui_callbacks["tile_bank_arrow_right"] = callbacks::arrow_right;
    ui_callbacks["main_area"] = callbacks::main_area;
    ui_callbacks["texture_area"] = callbacks::texture_area;

    std::vector<Tilemap> tilemaps = config::load_textures(config);

    SetTargetFPS(60);


    Camera2D main_camera = {};
    main_camera.zoom = 1.0f;
    Camera2D texture_camera = {};
    texture_camera.zoom = 1.0f;

    const int tile_size = config["tile_size_px"].as<int>();
    const Grid main_grid{.x_square_count = config["main_grid"]["count_x"].as<int>(),
                         .y_square_count = config["main_grid"]["count_y"].as<int>(),
                         .square_size_px = tile_size * config["main_grid"]["initial_scale"].as<int>()};

    const int initial_scale = config["texture_grid"]["initial_scale"].as<int>();
    const int margin = config["texture_grid"]["margin"].as<int>();

    Grid texture_grid{};
    if(tilemaps.size() > 0)
    {
	texture_grid = {.x_square_count = tilemaps[0].texture.width / tile_size + 2*margin,
	    .y_square_count = tilemaps[0].texture.height / tile_size + 2*margin,
	    .square_size_px = tile_size * initial_scale};
    }

    AppState app_state{.main_grid = main_grid, .texture_grid = texture_grid, .main_camera = main_camera, .texture_camera = texture_camera, .tilemap_index = {}};
    std::optional<std::string> previously_hovered_item{std::nullopt};


    while (!WindowShouldClose())
    {
        const Inputs inputs = get_inputs();
	const std::optional<std::string> hovered_item = get_ui_interaction(inputs, layers, ui);
	if(hovered_item.has_value())
	{
	    if(ui_callbacks.contains(hovered_item.value()))
	    {
		ui_callbacks[hovered_item.value()](inputs, ui, app_state, true);
	    }
	}
	else if(previously_hovered_item.has_value())
	{
	    if(ui_callbacks.contains(previously_hovered_item.value()))
	    {
		ui_callbacks[previously_hovered_item.value()](inputs, ui, app_state, false);
	    }
	}
	previously_hovered_item = hovered_item;




        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(app_state.main_camera);
	drawing::draw_main_area(app_state);
        EndMode2D();

	drawing::draw_ui(layers, ui);

        const int sc_x = screen_width * 0.025f;
        const int sc_y = screen_height * 0.125f;
        const int sc_w = screen_width * 0.29f;
        const int sc_h = screen_height * 0.49f;

        BeginScissorMode(sc_x, sc_y, sc_w, sc_h);
        BeginMode2D(app_state.texture_camera);

	drawing::draw_texture_area(app_state, tilemaps[app_state.tilemap_index].texture, config);
	
        EndMode2D();
        EndScissorMode();

        EndDrawing();
    }

    for (const auto &tilemap : tilemaps)
    {
        UnloadTexture(tilemap.texture);
    }
    CloseWindow();
    return 0;
}
