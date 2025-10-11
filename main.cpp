#include <array>
#include <cassert>
#include <string>
#include <map>
#include <variant>
#include <optional>
#include <iostream>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
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

struct Textbox
{
  Box box;
  Text text;
};

using Item = std::variant<Box, Triangle, Text, Textbox>;

}

struct Tilemap
{
  std::string texture_filename;
  Texture2D texture;
  std::vector<Rectangle> tiles;
};

template<typename T>
std::array<T, 2> get_screen_size(const YAML::Node &config)
{
    if(config["screen"]["fullscreen"].as<bool>())
    { 
      const int monitor = GetCurrentMonitor();
      return { static_cast<T>(GetMonitorWidth(monitor)), static_cast<T>(GetMonitorHeight(monitor)) };
    }

    return { config["screen"]["width"].as<T>(), config["screen"]["height"].as<T>()};
}

std::map<std::string, UI::Item> load_interface(const YAML::Node &config)
{
    const float screen_width = static_cast<float>(GetScreenWidth());
    const float screen_height = static_cast<float>(GetScreenHeight());
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
	else if(item_type == "textbox")
	{
	  const float box_width = screen_width * item["width"].as<float>();
	  const float box_height = screen_height * item["height"].as<float>();
	  const std::string text = item["text"].as<std::string>();
	  const float text_margin = item["text_margin"].as<float>();

	  int font_size = 1;
	  bool font_size_found = false;

	  while(not font_size_found)
	  {
	    const float text_width = static_cast<float>(MeasureText(text.c_str(), font_size));
	    if(text_width > (box_width - (box_width*text_margin)) )
	    {
	      // take previous font size
	      font_size_found = true;
	    }
	    else
	    {
	      font_size++;
	    }
	  }

	  const float box_x = screen_width * item["position_x"].as<float>();
	  const float box_y = screen_height * item["position_y"].as<float>();

	  const float text_width = static_cast<float>(MeasureText(text.c_str(), font_size));
	  const int text_x = static_cast<int>((box_width - text_width)/2.f + box_x);
	  const int text_y = static_cast<int>((box_height - static_cast<float>(font_size))/2.f + box_y);

	  map[item_key] = UI::Textbox{
	    UI::Box{
	      Rectangle{ .x = box_x,
		.y = box_y,
		.width = box_width,
		.height = box_height},
		color},
	      UI::Text{
		  .x = text_x,
		  .y = text_y,
		  .size = font_size,
		  .text = text,
		  .color = WHITE}};
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
    const float tile_size = config["tile_size"].as<float>();
    const float position_x = config["tile_bank"]["position_x"].as<float>();
    const float position_y = config["tile_bank"]["position_y"].as<float>();
    const unsigned tilebank_array_x = config["tile_bank"]["count_x"].as<unsigned>();
    const unsigned tilebank_array_y = config["tile_bank"]["count_y"].as<unsigned>();
    const float gap = config["tile_bank"]["gap"].as<float>();
    const float scale = config["tile_bank"]["scale"].as<float>();

    std::vector<Rectangle> tilebank_array;
    tilebank_array.reserve(tilebank_array_x * tilebank_array_y);

    const float offset_x = position_x * static_cast<float>(GetScreenWidth());
    const float offset_y = position_y * static_cast<float>(GetScreenHeight());
    
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

    InitWindow(0, 0, config["window_name"].as<std::string>().c_str());
    while(not IsWindowReady()) { };
    ToggleBorderlessWindowed();
    const auto [screen_width, screen_height] = get_screen_size<int>(config);
    SetWindowSize(screen_width, screen_height);

    std::map<std::string, UI::Item> interface = load_interface(config);
    std::vector<Rectangle> tilebank_array = load_tilebank_array(config);




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

    SetTargetFPS(60); 

    Color original_reload_button_color = std::get<UI::Textbox>(interface["reload_button"]).box.color;
    Color original_left_arrow_color = std::get<UI::Triangle>(interface["tile_bank_arrow_left"]).color;
    Color original_right_arrow_color = std::get<UI::Triangle>(interface["tile_bank_arrow_right"]).color;

    unsigned long tilemap_index = 0;


    Camera2D main_camera = {};
    main_camera.zoom = 1.0f;
    Camera2D texture_camera = {};
    texture_camera.zoom = 1.0f;


    while (!WindowShouldClose())
    {
	mousePoint = GetMousePosition();

	UI::Box &reload_button = std::get<UI::Textbox>(interface["reload_button"]).box;
	UI::Box &tile_bank = std::get<UI::Box>(interface["tile_bank"]);
	UI::Triangle &left_arrow = std::get<UI::Triangle>(interface["tile_bank_arrow_left"]);
	UI::Triangle &right_arrow = std::get<UI::Triangle>(interface["tile_bank_arrow_right"]);
	reload_button.color = original_reload_button_color;
	left_arrow.color = original_left_arrow_color;
	right_arrow.color = original_right_arrow_color;

	const int map_grid_gap = config["map"]["grid_gap_px"].as<int>();
	const int map_grid_x= config["map"]["grid_x"].as<int>();
	const int map_grid_y= config["map"]["grid_y"].as<int>();


	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
	  if (CheckCollisionPointRec(mousePoint, tile_bank.rectangle))
	  {
	    Vector2 delta = GetMouseDelta();
	    delta = Vector2Scale(delta, -1.0f/texture_camera.zoom);
	    texture_camera.target = Vector2Add(texture_camera.target, delta);
	    texture_camera.target = Vector2Clamp(texture_camera.target, {.x = -50.f, .y = -50.f}, {.x = static_cast<float>(map_grid_x*map_grid_gap)+50.f, .y = static_cast<float>(map_grid_y*map_grid_gap)+50.f});
	  }
	  else
	  {
	    Vector2 delta = GetMouseDelta();
	    delta = Vector2Scale(delta, -1.0f/main_camera.zoom);
	    main_camera.target = Vector2Add(main_camera.target, delta);
	    main_camera.target = Vector2Clamp(main_camera.target, {.x = -50.f, .y = -50.f}, {.x = static_cast<float>(map_grid_x*map_grid_gap)+50.f, .y = static_cast<float>(map_grid_y*map_grid_gap)+50.f});
	  }
	}

	const float wheel = GetMouseWheelMove();
	if (wheel != 0)
	{
	  if (CheckCollisionPointRec(mousePoint, tile_bank.rectangle))
	  {
	    const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), texture_camera);
	    texture_camera.offset = GetMousePosition();
	    texture_camera.target = mouseWorldPos;
	    const float scale = 0.2f*wheel;
	    texture_camera.zoom = Clamp(expf(logf(texture_camera.zoom)+scale), 0.125f, 64.0f);
	  }
	  else
	  {
	    const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), main_camera);
	    main_camera.offset = GetMousePosition();
	    main_camera.target = mouseWorldPos;
	    const float scale = 0.2f*wheel;
	    main_camera.zoom = Clamp(expf(logf(main_camera.zoom)+scale), 0.125f, 64.0f);
	  }
	}




	if (CheckCollisionPointRec(mousePoint, reload_button.rectangle))
	{
	    reload_button.color.a += 40;
	    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	    {
		config = YAML::LoadFile(config_path);
		interface = load_interface(config);
		tilebank_array = load_tilebank_array(config);
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

		UI::Text &text = std::get<UI::Text>(interface["tilemap_filename"]);
		text.text = tilemaps[tilemap_index].texture_filename;
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
		UI::Text &text = std::get<UI::Text>(interface["tilemap_filename"]);
		text.text = tilemaps[tilemap_index].texture_filename;
	    }
	}

	BeginDrawing();

	ClearBackground(RAYWHITE);

	BeginMode2D(main_camera);

	for(int i = 0; i < map_grid_x; i++)
	{
	  for(int j = 0; j < map_grid_y; j++)
	  {
	    DrawRectangleLines(i*map_grid_gap, j*map_grid_gap, map_grid_gap, map_grid_gap, BLACK);
	  }
	}
	DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 50, MAROON);

	EndMode2D();


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
	    else if(std::holds_alternative<UI::Textbox>(item.second))
	    {
		const auto &box = std::get<UI::Textbox>(item.second).box;
		DrawRectangleRec(box.rectangle, box.color);

		const auto &text = std::get<UI::Textbox>(item.second).text;
		DrawText(text.text.c_str(), text.x, text.y, text.size, text.color);
	    }
	}

	/*
	for(unsigned i = 0; i < tilebank_array.size(); i++)
	{
	  if(i == tilemaps[tilemap_index].tiles.size())
	  {
	    break;
	  }
	  DrawTexturePro(tilemaps[tilemap_index].texture, tilemaps[tilemap_index].tiles[i], tilebank_array[i], {0.f, 0.f}, 0.f, WHITE);
	}
	*/

	Vector2 screen_params = {static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
	const int sc_x = static_cast<int>(screen_params.x * 0.025f);
	const int sc_y = static_cast<int>(screen_params.y * 0.125f);
	const int sc_w = static_cast<int>(screen_params.x * 0.29f);
	const int sc_h = static_cast<int>(screen_params.y * 0.49f);


	BeginScissorMode(sc_x, sc_y, sc_w, sc_h);
	BeginMode2D(texture_camera);

	std::optional<Rectangle> highlighted{std::nullopt};
	Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), texture_camera);

	int gap = 16*3;
	for(int i = 0; i < tilemaps[tilemap_index].texture.width/16 + 2; i++)
	{
	  for(int j = 0; j < tilemaps[tilemap_index].texture.height/16 + 2; j++)
	  {
	    if(CheckCollisionPointRec(mouseWorldPos, Rectangle{.x=static_cast<float>(i*gap), .y=static_cast<float>(j*gap), .width=static_cast<float>(gap), .height=static_cast<float>(gap)}))
	    {
	      highlighted = {.x=static_cast<float>(i*gap), .y=static_cast<float>(j*gap), .width=static_cast<float>(gap), .height=static_cast<float>(gap)};
	    }
	    else
	    {
	      DrawRectangleLines(i*gap, j*gap, gap, gap, BLACK);
	    }
	  }
	}

	DrawTextureEx(tilemaps[tilemap_index].texture, {16.0f*3.f, 16.0f*3.f}, 0, 3, WHITE);
	if(highlighted)
	{
	  Color highlight = BLUE;
	  highlight.a = 100;
	  DrawRectangleRec(highlighted.value(), highlight);
	}

	EndMode2D();
	EndScissorMode();


	EndDrawing();
    }
    for(const auto &tilemap: tilemaps)
    {
      UnloadTexture(tilemap.texture);
    }
    CloseWindow();
    return 0;
}
