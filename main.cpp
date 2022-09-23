#include "GreekBot.h"
#include <iostream>
int main(int argc, const char** argv) {
	using namespace std::chrono;

	auto now = system_clock::now();
	auto nds = cDiscordClock::now();
	std::cout << "Seconds: " << (time_point_cast<milliseconds>(now).time_since_epoch() - nds.time_since_epoch()).count();

	cGreekBot(argv[1]).Run();
	return 0;
}