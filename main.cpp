#include "Database.h"
#include "GreekBot.h"

int main(int argc, const char** argv) {
	cDatabase::Initialize();
	cGreekBot(argv[1]).Run();
	return 0;
}