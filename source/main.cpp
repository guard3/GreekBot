#include <curl/curl.h>
#include <rapidjson/document.h>
#include <string>
#include <iostream>

static size_t cb(void* data, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	char* received = reinterpret_cast<char*>(malloc(realsize + 1));
	if (received) {
		memcpy(received, data, realsize);
		received[realsize] = '\0';

		std::string& result = *reinterpret_cast<std::string*>(userp);
		result += received;

		free(received);
		return realsize;
	}
	return 0;
}

int main() {
	curl_global_init(CURL_GLOBAL_DEFAULT);

	std::string endpoint;
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://discord.com/api/gateway");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &endpoint);
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	rapidjson::Document document;
	document.Parse(endpoint.c_str());
	std::cout << "Gateway endpoint: " << document["url"].GetString() << std::endl;

	curl_global_cleanup();
	return 0;
}