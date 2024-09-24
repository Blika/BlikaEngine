#pragma once

#include "model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace blikaengine{

	struct TransformComponent{
		glm::vec3 translation;
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation{};
		glm::mat4 mat4();
		glm::mat3 normal_matrix();
	};

	struct PointLightComponent{
		float light_intensity = 1.f;
	};

	class GameObject{
		public:
			using id_t = unsigned int;
			using Map = std::unordered_map<id_t, GameObject>;

			static GameObject create_game_object(){
				static id_t current_id = 0;
				return GameObject{current_id++};
			}

			static GameObject make_point_light(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

			GameObject(const GameObject&) = delete;
			GameObject& operator=(const GameObject&) = delete;
			GameObject(GameObject&&) = default;
			GameObject& operator=(GameObject&&) = default;

			id_t getId(){
				return id;
			}

			std::shared_ptr<Model> model{};
			glm::vec3 color{}; 
			TransformComponent transform{};

			std::unique_ptr<PointLightComponent> point_light = nullptr;
			glm::vec4 multipliers{1,1,1,1};
			short next_color = 0;
		private:

			GameObject(id_t obj_id): id{obj_id}{}

			id_t id;
	};
}