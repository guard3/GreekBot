#include "Database.h"
#include "GreekBot.h"
#include <cstdio>

int main(int argc, const char** argv) {
	/* Check arguments! */
	if (argc != 2) {
		std::puts("You forgot to give me a token!");
		return EXIT_FAILURE;
	}
	/* Initialize the database and the bot client */
	cDatabase::Initialize();
	cGreekBot client(argv[1]);
	client.Run();

	return EXIT_SUCCESS;
}