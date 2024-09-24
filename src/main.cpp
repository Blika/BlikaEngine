#include "blikaengine.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(){
	blikaengine::BlikaEngine be{};
	try{
		be.run();
	}catch(const std::exception& e){
		std::cerr << e.what() << '\n';
		//system("pause");
		return EXIT_FAILURE;
	}
		//system("pause");
	return EXIT_SUCCESS;
}