#include "model.hpp"
#include "blikaengine.hpp"
#include "utils/utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>
#include <functional>
#include <future>
#include <thread>

namespace std{
	template<>
	struct hash<blikaengine::Model::Vertex>{
		size_t operator()(blikaengine::Model::Vertex const& vertex) const{
			size_t seed = 0;
			blikaengine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace blikaengine{
	
	Model::Model(Device& device, const Model::Data& data) : device{device} {
		create_vertex_buffers(data.vertices);
		create_index_buffer(data.indices);
	}

	Model::~Model(){
	}

	std::unique_ptr<Model> Model::create_model_from_file(Device& device, const std::string& filepath){
		Data data{};
		data.load_model(filepath);
		return std::make_unique<Model>(device, data);
	}

	void Model::create_vertex_buffers(const std::vector<Vertex>& vertices){
		vertex_count = static_cast<uint32_t>(vertices.size());
		assert(vertex_count >= 3 && "vertex count must be at least 3");
		uint32_t vertex_size = sizeof(vertices[0]);
		VkDeviceSize buffer_size = vertex_size * vertex_count;
		Buffer staging_buffer{device,vertex_size,vertex_count,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
		staging_buffer.map();
		staging_buffer.writeToBuffer((void*) vertices.data());
		vertex_buffer = std::make_unique<Buffer>(device,vertex_size,vertex_count,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		device.copyBuffer(staging_buffer.getBuffer(), vertex_buffer->getBuffer(), buffer_size);
	}
	
	void Model::create_index_buffer(const std::vector<uint32_t>& indices){
		index_count = static_cast<uint32_t>(indices.size());
		has_index_buffer = index_count > 0;
		if(!has_index_buffer){
			return;
		}
		uint32_t index_size = sizeof(indices[0]);
		VkDeviceSize buffer_size = index_size * index_count;
		Buffer staging_buffer{device,index_size,index_count,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
		staging_buffer.map();
		staging_buffer.writeToBuffer((void*) indices.data());
		index_buffer = std::make_unique<Buffer>(device,index_size,index_count,VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		device.copyBuffer(staging_buffer.getBuffer(), index_buffer->getBuffer(), buffer_size);
	}

	void Model::draw(VkCommandBuffer command_buffer){
		if(has_index_buffer){
			vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
		}else{
			vkCmdDraw(command_buffer,vertex_count,1,0,0);
		}
	}

	void Model::bind(VkCommandBuffer command_buffer){
		VkBuffer buffers[] = {vertex_buffer->getBuffer()};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(command_buffer,0,1,buffers,offsets);
		if(has_index_buffer){
			vkCmdBindIndexBuffer(command_buffer, index_buffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32); //uint16 for light models maybe?
		}
	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::get_binding_descriptions(){
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
		binding_descriptions[0].binding = 0;
		binding_descriptions[0].stride = sizeof(Vertex);
		binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::get_attribute_descriptions(){
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

		attribute_descriptions.push_back({0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, position)});
		attribute_descriptions.push_back({1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, color)});
		attribute_descriptions.push_back({2,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, normal)});
		attribute_descriptions.push_back({3,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, uv)});
		return attribute_descriptions;
	}
	
	void Model::Data::load_model(const std::string& filepath){
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())){
			throw std::runtime_error(warn + err);
		}
		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> unique_vertices{};
		for(const auto& shape : shapes){
			for(const auto& index : shape.mesh.indices){
				Vertex vertex{};
				if(index.vertex_index >= 0){
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};
					
					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
				}
				if(index.normal_index >= 0){
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				if(index.texcoord_index >= 0){
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if(unique_vertices.count(vertex) == 0){
					unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(unique_vertices[vertex]);
			}
		}
	}
}