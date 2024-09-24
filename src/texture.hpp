#pragma once

#include "device.hpp"

#include <stb_image.h>
#define STB_IMAGE_IMPLEMENTATION

#include <string>
#include <vulkan/vulkan_core.h>

namespace blikaengine{
    class Texture{
        public:
            Texture(Device& device, const std::string& filepath);
            ~Texture();

            Texture(const Texture&) = delete;
            Texture &operator=(const Texture&) = delete;
            Texture(Texture&&) = delete;
            Texture &operator=(Texture&&) = delete;

            VkSampler get_sampler(){
                return sampler;
            }

            VkImageView get_image_view(){
                return image_view;
            }

            VkImageLayout get_image_layout(){
                return image_layout;
            }
        private:
            void transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout);
            void generate_mipmaps();

            int width, height, mip_levels;

            Device& device;
            VkImage image;
            VkDeviceMemory image_memory;
            VkImageView image_view;
            VkSampler sampler;
            VkFormat image_format;
            VkImageLayout image_layout;
    };
}