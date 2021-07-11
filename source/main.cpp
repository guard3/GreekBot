#include <curl/curl.h>
#include <rapidjson/rapidjson.h>

int main() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "https://discord.com/api/gateway");
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return 0;
}