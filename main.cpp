#include <array>
#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <variant>
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

} // namespace UI

struct Tilemap
{
    std::string texture_filename;
    Texture2D texture;
    std::vector<Rectangle> tiles;
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

MouseButtonState get_mouse_button_state(const MouseButton button)
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

Inputs get_inputs()
{
    return {GetMousePosition(), get_mouse_button_state(MOUSE_BUTTON_LEFT), get_mouse_button_state(MOUSE_BUTTON_RIGHT),
            GetMouseWheelMove()};
}

struct Grid
{
    int x_square_count{};
    int y_square_count{};
    int square_size{};
};

struct GameState
{
    Grid main_grid{};
    Grid texture_grid{};
    Camera2D main_camera{};
    Camera2D texture_camera{};

    UI::Box tile_bank{};
};

std::array<Vector2, 2> get_camera_boundaries(const Grid &grid)
{
    const Vector2 min{-grid.square_size, -grid.square_size};
    const Vector2 max{(grid.x_square_count + 1) * grid.square_size, (grid.y_square_count + 1) * grid.square_size};
    return {min, max};
}

void pan_camera(Camera2D &camera, const Vector2 &clamp_min, const Vector2 &clamp_max)
{
    const Vector2 delta = Vector2Scale(GetMouseDelta(), -1.0f / camera.zoom);
    camera.target = Vector2Clamp(Vector2Add(camera.target, delta), clamp_min, clamp_max);
}

void zoom_camera(Camera2D &camera, const Inputs &inputs)
{
    const float scale = 0.2f * inputs.wheel;
    const Vector2 mouseWorldPos = GetScreenToWorld2D(inputs.mouse_point, camera);
    camera.offset = inputs.mouse_point;
    camera.target = mouseWorldPos;
    camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
}

void update_game_state(const Inputs &inputs, GameState &game_state)
{
    if ((inputs.right_mouse_button == MouseButtonState::DOWN) or
        (inputs.right_mouse_button == MouseButtonState::PRESSED))
    {
        if (CheckCollisionPointRec(inputs.mouse_point, game_state.tile_bank.rectangle))
        {
            const auto [min, max] = get_camera_boundaries(game_state.texture_grid);
            pan_camera(game_state.texture_camera, min, max);
        }
        else
        {
            const auto [min, max] = get_camera_boundaries(game_state.main_grid);
            pan_camera(game_state.main_camera, min, max);
        }
    }

    if (inputs.wheel != 0)
    {
        if (CheckCollisionPointRec(inputs.mouse_point, game_state.tile_bank.rectangle))
        {
            zoom_camera(game_state.texture_camera, inputs);
        }
        else
        {
            zoom_camera(game_state.main_camera, inputs);
        }
    }
};

template <typename T> std::array<T, 2> get_screen_size(const YAML::Node &config)
{
    if (config["screen"]["fullscreen"].as<bool>())
    {
        const int monitor = GetCurrentMonitor();
        return {static_cast<T>(GetMonitorWidth(monitor)), static_cast<T>(GetMonitorHeight(monitor))};
    }

    return {config["screen"]["width"].as<T>(), config["screen"]["height"].as<T>()};
}

std::map<std::string, UI::Item> load_interface(const YAML::Node &config)
{
    const float screen_width = GetScreenWidth();
    const float screen_height = GetScreenHeight();
    std::map<std::string, UI::Item> map{};
    for (const auto &item_pair : config["interface"])
    {
        const std::string item_key = item_pair.first.as<std::string>();
        const auto &item = item_pair.second;
        const std::string item_type = item["type"].as<std::string>();
        const Color color = {item["color"]["r"].as<unsigned char>(), item["color"]["g"].as<unsigned char>(),
                             item["color"]["b"].as<unsigned char>(), item["color"]["a"].as<unsigned char>()};

        if (item_type == "box")
        {
            map[item_key] = UI::Box{Rectangle{.x = screen_width * item["position_x"].as<float>(),
                                              .y = screen_height * item["position_y"].as<float>(),
                                              .width = screen_width * item["width"].as<float>(),
                                              .height = screen_height * item["height"].as<float>()},
                                    color};
        }
        else if (item_type == "triangle")
        {
            map[item_key] =
                UI::Triangle{.p1 = {screen_width * item["p1x"].as<float>(), screen_width * item["p1y"].as<float>()},
                             .p2 = {screen_width * item["p2x"].as<float>(), screen_width * item["p2y"].as<float>()},
                             .p3 = {screen_width * item["p3x"].as<float>(), screen_width * item["p3y"].as<float>()},
                             .color = color};
        }
        else if (item_type == "text")
        {
            map[item_key] = UI::Text{.x = screen_width * item["position_x"].as<float>(),
                                     .y = screen_height * item["position_y"].as<float>(),
                                     .size = item["font_size"].as<int>(),
                                     .text = item["text"].as<std::string>(),
                                     .color = color};
        }
        else if (item_type == "textbox")
        {
            const float box_width = screen_width * item["width"].as<float>();
            const float box_height = screen_height * item["height"].as<float>();
            const std::string text = item["text"].as<std::string>();
            const float text_margin = item["text_margin"].as<float>();

            int font_size = 1;
            bool font_size_found = false;

            while (not font_size_found)
            {
                const float text_width = MeasureText(text.c_str(), font_size);
                if (text_width > (box_width - (box_width * text_margin)))
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

            const float text_width = MeasureText(text.c_str(), font_size);
            const int text_x = (box_width - text_width) / 2.f + box_x;
            const int text_y = (box_height - font_size) / 2.f + box_y;

            map[item_key] =
                UI::Textbox{UI::Box{Rectangle{.x = box_x, .y = box_y, .width = box_width, .height = box_height}, color},
                            UI::Text{.x = text_x, .y = text_y, .size = font_size, .text = text, .color = WHITE}};
        }
        else
        {
            assert(false);
        }
    }

    return map;
}

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

int main(void)
{
    const std::string config_path = "../resources/config.yaml";

    YAML::Node config = YAML::LoadFile(config_path);

    InitWindow(0, 0, config["window_name"].as<std::string>().c_str());
    while (not IsWindowReady())
    {
    };
    ToggleBorderlessWindowed();
    const auto [screen_width, screen_height] = get_screen_size<int>(config);
    SetWindowSize(screen_width, screen_height);

    std::map<std::string, UI::Item> interface = load_interface(config);
    std::vector<Rectangle> tilebank_array = load_tilebank_array(config);

    // Texture load
    const int tile_size = config["tile_size"].as<int>();
    std::vector<Tilemap> tilemaps{};
    tilemaps.reserve(config["tile_filenames"].size());
    for (const auto &filename : config["tile_filenames"])
    {
        const std::string texture_filename =
            "../" + config["asset_path"].as<std::string>() + "/" + filename.as<std::string>();
        const Texture2D texture = LoadTexture(texture_filename.c_str());
        assert((texture.width % tile_size) == 0);
        assert((texture.height % tile_size) == 0);
        const int tiles_count_x = texture.width / tile_size;
        const int tiles_count_y = texture.height / tile_size;
        std::vector<Rectangle> tiles;
        tiles.reserve(static_cast<unsigned>(tiles_count_x * tiles_count_y));
        for (int i = 0; i < (tiles_count_x * tiles_count_y); i++)
        {
            tiles.push_back({.x = tile_size * (i % tiles_count_x),
                             .y = tile_size * (i / tiles_count_x),
                             .width = tile_size,
                             .height = tile_size});
        }
        tilemaps.push_back(Tilemap{std::move(texture_filename), texture, std::move(tiles)});
    }

    SetTargetFPS(60);

    Color original_reload_button_color = std::get<UI::Textbox>(interface["reload_button"]).box.color;
    Color original_left_arrow_color = std::get<UI::Triangle>(interface["tile_bank_arrow_left"]).color;
    Color original_right_arrow_color = std::get<UI::Triangle>(interface["tile_bank_arrow_right"]).color;

    unsigned long tilemap_index = 0;

    Camera2D main_camera = {};
    main_camera.zoom = 1.0f;
    Camera2D texture_camera = {};
    texture_camera.zoom = 1.0f;

    const Grid main_grid{.x_square_count = config["map"]["grid_x"].as<int>(),
                         .y_square_count = config["map"]["grid_y"].as<int>(),
                         .square_size = config["map"]["grid_gap_px"].as<int>()};

    // TODO: FIX THIS add to config or something
    const Grid texture_grid{.x_square_count = tilemaps[tilemap_index].texture.width / 16 + 2,
                            .y_square_count = tilemaps[tilemap_index].texture.height / 16 + 2,
                            .square_size = 16 * 3};

    GameState state{.main_grid = main_grid,
                    .texture_grid = texture_grid,
                    .main_camera = main_camera,
                    .texture_camera = texture_camera,
                    .tile_bank = std::get<UI::Box>(interface["tile_bank"])};

    while (!WindowShouldClose())
    {

        const Inputs inputs = get_inputs();
        update_game_state(inputs, state);

        UI::Box &reload_button = std::get<UI::Textbox>(interface["reload_button"]).box;
        UI::Triangle &left_arrow = std::get<UI::Triangle>(interface["tile_bank_arrow_left"]);
        UI::Triangle &right_arrow = std::get<UI::Triangle>(interface["tile_bank_arrow_right"]);
        reload_button.color = original_reload_button_color;
        left_arrow.color = original_left_arrow_color;
        right_arrow.color = original_right_arrow_color;

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

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(state.main_camera);

        for (int i = 0; i < state.main_grid.x_square_count; i++)
        {
            for (int j = 0; j < state.main_grid.y_square_count; j++)
            {
                const int square_size = state.main_grid.square_size;
                DrawRectangleLines(i * square_size, j * square_size, square_size, square_size, BLACK);
            }
        }
        DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 50, MAROON);

        EndMode2D();

        for (const auto &item : interface)
        {
            if (std::holds_alternative<UI::Box>(item.second))
            {
                const auto &box = std::get<UI::Box>(item.second);
                DrawRectangleRec(box.rectangle, box.color);
            }
            else if (std::holds_alternative<UI::Triangle>(item.second))
            {
                const auto &triangle = std::get<UI::Triangle>(item.second);
                DrawTriangle(triangle.p1, triangle.p2, triangle.p3, triangle.color);
            }
            else if (std::holds_alternative<UI::Text>(item.second))
            {
                const auto &text = std::get<UI::Text>(item.second);
                DrawText(text.text.c_str(), text.x, text.y, text.size, text.color);
            }
            else if (std::holds_alternative<UI::Textbox>(item.second))
            {
                const auto &box = std::get<UI::Textbox>(item.second).box;
                DrawRectangleRec(box.rectangle, box.color);

                const auto &text = std::get<UI::Textbox>(item.second).text;
                DrawText(text.text.c_str(), text.x, text.y, text.size, text.color);
            }
        }

        Vector2 screen_params = {GetScreenWidth(), GetScreenHeight()};
        const int sc_x = screen_params.x * 0.025f;
        const int sc_y = screen_params.y * 0.125f;
        const int sc_w = screen_params.x * 0.29f;
        const int sc_h = screen_params.y * 0.49f;

        BeginScissorMode(sc_x, sc_y, sc_w, sc_h);
        BeginMode2D(state.texture_camera);

        std::optional<Rectangle> highlighted{std::nullopt};
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), state.texture_camera);

        for (int i = 0; i < state.texture_grid.x_square_count; i++)
        {
            for (int j = 0; j < state.texture_grid.y_square_count; j++)
            {
                const int square_size = state.texture_grid.square_size;
                if (CheckCollisionPointRec(mouseWorldPos, Rectangle{.x = i * square_size,
                                                                    .y = j * square_size,
                                                                    .width = square_size,
                                                                    .height = square_size}))
                {
                    highlighted = {
                        .x = i * square_size, .y = j * square_size, .width = square_size, .height = square_size};
                }
                else
                {
                    DrawRectangleLines(i * square_size, j * square_size, square_size, square_size, BLACK);
                }
            }
        }

        DrawTextureEx(tilemaps[tilemap_index].texture, {16.0f * 3.f, 16.0f * 3.f}, 0, 3, WHITE);
        if (highlighted)
        {
            Color highlight = BLUE;
            highlight.a = 100;
            DrawRectangleRec(highlighted.value(), highlight);
        }

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
