#pragma once

#include "buffer.hpp"
#include "device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace blikaengine{

	class Model{
		public:
			struct Vertex{
				glm::vec3 position{};
				glm::vec3 color{};
				glm::vec3 normal{};
				glm::vec2 uv{};

				static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
				static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

				bool operator==(const Vertex& other)const{
					return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
				}
			};

			struct Data{
				std::vector<Vertex> vertices{};
				std::vector<uint32_t> indices{};

				void load_model(const std::string& filepath);
			};

			Model(Device& device, const Model::Data& data);
			~Model();
			Model(const Model&) = delete;
			Model& operator = (const Model&) = delete;
			
			void create_texture_image();

			static std::unique_ptr<Model> create_model_from_file(Device& device, const std::string& filepath);

			void bind(VkCommandBuffer command_buffer);

			void draw(VkCommandBuffer command_buffer);

		private:
			
			void create_vertex_buffers(const std::vector<Vertex>& vertices);
			void create_index_buffer(const std::vector<uint32_t>& indices);

			Device& device;
			std::unique_ptr<Buffer> vertex_buffer;
			uint32_t vertex_count;
			bool has_index_buffer = false;
			std::unique_ptr<Buffer> index_buffer;
			uint32_t index_count;

	};
}