#include <cassert>
#include <string>
#include <map>
#include <variant>
#include <iostream>
#include "raylib.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/yaml.h"

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

using Item = std::variant<Box, Triangle, Text>;

}

struct Tilemap
{
  std::string texture_filename;
  Texture2D texture;
  std::vector<Rectangle> tiles;
};

std::map<std::string, UI::Item> load_interface(const YAML::Node &config)
{
    const float screen_width = config["screen"]["width"].as<float>();
    const float screen_height = config["screen"]["height"].as<float>();

    std::map<std::string, UI::Item> map{};
    for(const auto &item_pair : config["interface"])
    {
	const std::string item_key = item_pair.first.as<std::string>();
	const auto &item = item_pair.second;
	const std::string item_type = item["type"].as<std::string>();
	const Color color = {item["color"]["r"].as<unsigned char>(),
	    item["color"]["g"].as<unsigned char>(), item["color"]["b"].as<unsigned char>(),
	    item["color"]["a"].as<unsigned char>()
	};

	std::cout << item_key << "\n";
	if(item_type == "box")
	{
	    map[item_key] = UI::Box{
		Rectangle{ .x = screen_width * item["position_x"].as<float>(),
		    .y = screen_height * item["position_y"].as<float>(),
		    .width = screen_width * item["width"].as<float>(),
		    .height = screen_height * item["height"].as<float>()},
		    color};
	}
	else if(item_type == "triangle")
	{
	    map[item_key] = UI::Triangle{
		 .p1 = {screen_width * item["p1x"].as<float>(), screen_width * item["p1y"].as<float>()},
		 .p2 = {screen_width * item["p2x"].as<float>(), screen_width * item["p2y"].as<float>()},
		 .p3 = {screen_width * item["p3x"].as<float>(), screen_width * item["p3y"].as<float>()},
		    .color = color};

	}
	else if(item_type == "text")
	{
	    map[item_key] = UI::Text{
		.x = static_cast<int>(screen_width * item["position_x"].as<float>()),
		    .y = static_cast<int>(screen_height * item["position_y"].as<float>()),
		    .size= item["font_size"].as<int>(),
		    .text = item["text"].as<std::string>(),
		    .color = color};
	}
	else
	{
	    assert(false);
	}
    }

    std::cout << std::flush;
    return map;
}

std::vector<Rectangle> load_tilebank_array(const YAML::Node &config)
{
    const float screen_width = config["screen"]["width"].as<float>();
    const float screen_height = config["screen"]["height"].as<float>();
    const float tile_size = config["tile_size"].as<float>();
    const float position_x = config["tile_bank"]["position_x"].as<float>();
    const float position_y = config["tile_bank"]["position_y"].as<float>();
    const unsigned tilebank_array_x = config["tile_bank"]["count_x"].as<unsigned>();
    const unsigned tilebank_array_y = config["tile_bank"]["count_y"].as<unsigned>();
    const float gap = config["tile_bank"]["gap"].as<float>();
    const float scale = config["tile_bank"]["scale"].as<float>();

    std::vector<Rectangle> tilebank_array;
    tilebank_array.reserve(tilebank_array_x * tilebank_array_y);

    const float offset_x = position_x * screen_width;
    const float offset_y = position_y * screen_height;
    
    for(unsigned i = 0; i < (tilebank_array_x * tilebank_array_y); i++)
    {

      tilebank_array.push_back({.x = offset_x + static_cast<float>(i % tilebank_array_x) * ((scale * tile_size) + gap),
			      .y = offset_y +  static_cast<float>(i / tilebank_array_x )* ((scale * tile_size) + gap),
			      .width = scale*static_cast<float>(tile_size),
			      .height = scale*static_cast<float>(tile_size)});
    }
    return tilebank_array;
}

int main(void)
{
    const std::string config_path = "../resources/config.yaml";

    YAML::Node config = YAML::LoadFile(config_path);
    std::map<std::string, UI::Item> interface = load_interface(config);
    std::vector<Rectangle> tilebank_array = load_tilebank_array(config);

    const float screen_width = config["screen"]["width"].as<float>();
    const float screen_height = config["screen"]["height"].as<float>();

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    InitWindow(static_cast<int>(screen_width), static_cast<int>(screen_height), config["window_name"].as<std::string>().c_str());


    // Texture load
    const unsigned tile_size = config["tile_size"].as<unsigned>();
    std::vector<Tilemap> tilemaps{};
    tilemaps.reserve(config["tile_filenames"].size());
    for(const auto &filename: config["tile_filenames"])
    {
      const std::string texture_filename = "../" + config["asset_path"].as<std::string>() + "/" + filename.as<std::string>();
      const Texture2D texture = LoadTexture(texture_filename.c_str());
      assert((static_cast<unsigned>(texture.width) % tile_size) == 0);
      assert((static_cast<unsigned>(texture.height) % tile_size) == 0);
      const unsigned tiles_count_x = static_cast<unsigned>(texture.width) / tile_size;
      const unsigned tiles_count_y = static_cast<unsigned>(texture.height) / tile_size;
      std::vector<Rectangle> tiles;
      tiles.reserve(tiles_count_x * tiles_count_y);
      for(unsigned i = 0; i < (tiles_count_x * tiles_count_y); i++)
      {
	tiles.push_back({.x = static_cast<float>(tile_size * (i % tiles_count_x)),
	    .y = static_cast<float>(tile_size * (i / tiles_count_x)),
	    .width = static_cast<float>(tile_size),
	    .height = static_cast<float>(tile_size)});
      }
      tilemaps.push_back(Tilemap{std::move(texture_filename), texture, std::move(tiles)});
    }



    Vector2 mousePoint = { 0.0f, 0.0f };

    //SetConfigFlags(FLAG_VSYNC_HINT);
    SetTargetFPS(60); 

    Color original_reload_button_color = std::get<UI::Box>(interface["reload_button"]).color;
    Color original_left_arrow_color = std::get<UI::Triangle>(interface["tile_bank_arrow_left"]).color;
    Color original_right_arrow_color = std::get<UI::Triangle>(interface["tile_bank_arrow_right"]).color;

    unsigned long tilemap_index = 0;

    while (!WindowShouldClose())
    {
	mousePoint = GetMousePosition();

	UI::Box &reload_button = std::get<UI::Box>(interface["reload_button"]);
	UI::Triangle &left_arrow = std::get<UI::Triangle>(interface["tile_bank_arrow_left"]);
	UI::Triangle &right_arrow = std::get<UI::Triangle>(interface["tile_bank_arrow_right"]);
	reload_button.color = original_reload_button_color;
	left_arrow.color = original_left_arrow_color;
	right_arrow.color = original_right_arrow_color;

	if (CheckCollisionPointRec(mousePoint, reload_button.rectangle))
	{
	    reload_button.color.a += 40;
	    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	    {
		config = YAML::LoadFile(config_path);
		interface = load_interface(config);
		tilebank_array = load_tilebank_array(config);
		original_reload_button_color = std::get<UI::Box>(interface["reload_button"]).color;
	    }
	}
	else if (CheckCollisionPointTriangle(mousePoint, left_arrow.p1, left_arrow.p2, left_arrow.p3))
	{
	    left_arrow.color.a += 40;
	    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	    {
		if(tilemap_index == 0)
		{
		    tilemap_index = tilemaps.size() - 1;
		}
		else
		{
		    tilemap_index--;
		}

		UI::Text &text = std::get<UI::Text>(interface["asset_filename"]);
		text.text = tilemaps[tilemap_index].texture_filename;
		original_reload_button_color = std::get<UI::Box>(interface["reload_button"]).color;
	    }
	}
	else if (CheckCollisionPointTriangle(mousePoint, right_arrow.p1, right_arrow.p2, right_arrow.p3))
	{
	    right_arrow.color.a += 40;
	    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	    {
		tilemap_index++;
		if(tilemap_index == tilemaps.size())
		{
		    tilemap_index = 0;
		}
		UI::Text &text = std::get<UI::Text>(interface["asset_filename"]);
		text.text = tilemaps[tilemap_index].texture_filename;
		original_reload_button_color = std::get<UI::Box>(interface["reload_button"]).color;
	    }
	}

	BeginDrawing();

	ClearBackground(RAYWHITE);

	for(const auto &item: interface)
	{
	    if(std::holds_alternative<UI::Box>(item.second))
	    {
		const auto &box = std::get<UI::Box>(item.second);
		DrawRectangleRec(box.rectangle, box.color);
	    }
	    else if(std::holds_alternative<UI::Triangle>(item.second))
	    {
		const auto &triangle = std::get<UI::Triangle>(item.second);
		DrawTriangle(triangle.p1, triangle.p2, triangle.p3, triangle.color);
	    }
	    else if(std::holds_alternative<UI::Text>(item.second))
	    {
		const auto &text = std::get<UI::Text>(item.second);
		DrawText(text.text.c_str(), text.x, text.y, text.size, text.color);
	    }
	}
	for(unsigned i = 0; i < tilebank_array.size(); i++)
	{
	  if(i == tilemaps[tilemap_index].tiles.size())
	  {
	    break;
	  }
	  DrawTexturePro(tilemaps[tilemap_index].texture, tilemaps[tilemap_index].tiles[i], tilebank_array[i], {0.f, 0.f}, 0.f, WHITE);
	}
	EndDrawing();
    }
    for(const auto &tilemap: tilemaps)
    {
      UnloadTexture(tilemap.texture);
    }
    CloseWindow();
    return 0;
}
