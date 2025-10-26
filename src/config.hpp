#pragma once
#include <array>
#include "raylib.h"
#include "yaml-cpp/yaml.h"
#include "ui.hpp"

namespace config
{

inline std::array<int, 2> get_screen_size(const YAML::Node &config)
{
    if (config["screen"]["fullscreen"].as<bool>())
    {
        const int monitor = GetCurrentMonitor();
        return {GetMonitorWidth(monitor), GetMonitorHeight(monitor)};
    }

    return {config["screen"]["width"].as<int>(), config["screen"]["height"].as<int>()};
}

std::pair<std::vector<std::vector<std::string>>, std::map<std::string, UI::Item>>
load_interface(const YAML::Node &config, const float screen_width, const float screen_height)
{
    std::map<std::string, UI::Item> map{};
    std::vector<std::vector<std::string>> layers{};
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
        else if (item_type == "textbox")
        {
            const float box_width = screen_width * item["width"].as<float>();
            const float box_height = screen_height * item["height"].as<float>();
            const std::string text = item["text"].as<std::string>();
            const float text_margin = item["text_margin"].as<float>();

            // find font size which will fill the whole box
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
        else
        {
            printf("Type: %s not supported", item_type.c_str());
        }

        if (map.contains(item_key))
        {
            const unsigned layer = item["layer"].as<unsigned>();
            if (layers.size() < layer)
            {
                layers.resize(layer + 1);
            }

            layers[layer].push_back(item_key);
        }
    }
    return {layers, map};
}

inline std::vector<Tilemap> load_textures(const YAML::Node &config)
{
    const int tile_size = config["tile_size_px"].as<int>();
    std::vector<Tilemap> tilemaps{};
    tilemaps.reserve(config["tile_filenames"].size());
    for (const auto &filename : config["tile_filenames"])
    {
        const std::string texture_filename =
            "../" + config["asset_path"].as<std::string>() + "/" + filename.as<std::string>();
        const Texture2D texture = LoadTexture(texture_filename.c_str());
        assert((texture.width % tile_size) == 0);
        assert((texture.height % tile_size) == 0);
        tilemaps.push_back(Tilemap{std::move(texture_filename), texture});
    }

    return tilemaps;
}

} // namespace config
