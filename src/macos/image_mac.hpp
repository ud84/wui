#pragma once
#include <wui/common/error.hpp>
#include <cstdint>
#include <string_view>
#include <vector>
void load_image_from_data(const std::vector<uint8_t>& data, void **image);
void load_image_from_file(std::string_view file, std::string_view path, void **image, wui::error& err);
void free_image(void **image);
int32_t mac_image_width(void *image);
int32_t mac_image_height(void *image);
