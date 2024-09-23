#include "Database.h"
#include "GreekBot.h"
#include "Utils.h"
#include <csignal>
#include <thread>

int main(int argc, const char** argv) {
	/* Set the main thread to block certain signals */
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTSTP);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	/* Check arguments! */
	if (argc != 2) {
		fmt::print("You forgot to give me a token!\n");
		return EXIT_FAILURE;
	}
	/* Initialize the database and the bot client */
	cDatabase::Initialize();
	cGreekBot client(argv[1]);
	/* Wait for a termination signal in order to stop the bot gracefully */
	std::thread t(&cGreekBot::Run, &client);
	for (int sig = 0;;) {
		sigwait(&set, &sig);
		switch (sig) {
			case SIGINT:
			case SIGTSTP:
			case SIGTERM:
				cUtils::PrintErr("Interrupt received. Terminating...");
				client.RequestExit();
				break;
			default:
				continue;
		}
		break;
	}
	t.join();
	return EXIT_SUCCESS;
}