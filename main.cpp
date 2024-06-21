#include "Database.h"
#include "GreekBot.h"

int main(int argc, const char** argv) {
	cDatabase::Initialize();
	DiscordMain<cGreekBot>(argv[1]);
	return 0;
}