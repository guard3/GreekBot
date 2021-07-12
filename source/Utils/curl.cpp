#include <curl/curl.h>

/* A simple class that automatically handles libcurl initialization */
class curl_autoinit final {
private:
	static curl_autoinit m_instance;

	curl_autoinit() {
		CURLcode result;
		if ((result = curl_global_init(CURL_GLOBAL_DEFAULT)) != CURLE_OK)
			exit(result);
	}

public:
	~curl_autoinit() {
		curl_global_cleanup();
	}
} curl_autoinit::m_instance;