#include <iostream>
#include "Discord.h"


int main(int argc, const char** argv) {
	cGateway gateway(argv[1]);

	if (!gateway) {
		std::cout << gateway.GetError()->GetMessage() << std::endl;
	}
	else {
		std::cout << gateway.GetUrl() << std::endl;
	}

	return 0;
}