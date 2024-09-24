#pragma once

#include "descriptors.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "renderer.hpp"
#include "window.hpp"

#include <memory>
#include <random>
#include <vector>

namespace blikaengine{

	class BlikaEngine{
		public:
			static constexpr int WIDTH = 1200;
			static constexpr int HEIGHT = 800;
			static std::default_random_engine rnd_eng;
			static BlikaEngine* instance;
			
			BlikaEngine();
			~BlikaEngine();
			BlikaEngine(const BlikaEngine&) = delete;
			BlikaEngine& operator = (const BlikaEngine&) = delete;

			static BlikaEngine* get_instance(){
				return instance;
			}

			void run();

		private:
			void load_game_objects();

			Window window{WIDTH, HEIGHT, "Blika Engine"};
			Device device{window};
			Renderer renderer{window,device};

			std::unique_ptr<DescriptorPool> global_pool{};
			GameObject::Map game_objects;
	};

}