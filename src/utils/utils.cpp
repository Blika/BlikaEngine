#include "utils.hpp"
#include <random>

namespace blikaengine{
    
	float rnd(float min, float max){
		std::uniform_real_distribution<float> rnd_dist(min,max);
		return rnd_dist(BlikaEngine::rnd_eng);
	}

}