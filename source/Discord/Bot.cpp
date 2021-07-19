#include "Bot.h"
#include "Gateway.h"
#include <thread>
#include <chrono>

cBot::cBot(const char* token) {
	/* Add bot token to the http auth string */
	if (token) {
		strncpy(m_http_auth + 4, token, 59);
		m_http_auth[63] = '\0';
	}
}

void cBot::Run() {
	printf("Bot running...\n");
#if 0
	for (;;) {
		/* Get gateway info */
		cGateway gateway(m_http_auth);
		if (!gateway) {
			auto error = gateway.GetError();
			fprintf(stderr, "\nError retrieving gateway information\nCode: %d\n%s\n", error->GetCode(), error->GetMessage());
			break;
		}
		
		/* Connect to websocket */
		//TBA
		
		printf("%s\n", gateway.GetUrl());
		break;
		
	}
#endif
	printf("\nExiting...\n");
}
