#include "Discord.h"
#include <curl/curl.h>

struct sReceiveData {
	void*  data;
	size_t size;
};

static const char s_dummy = '\0';

static size_t cb(void* data, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	sReceiveData* receive_data = reinterpret_cast<sReceiveData*>(userp);
	char* temp = reinterpret_cast<char*>(realloc(receive_data->data, receive_data->size + realsize));
	if (temp) {
		memcpy(temp + receive_data->size, data, realsize);
		receive_data->data = temp;
		receive_data->size += realsize;
		return realsize;
	}
	return 0;
}

CURLcode cDiscord::GetGateway(const char* token, rapidjson::Document& document) {
	CURLcode result = CURLcode::CURLE_RECV_ERROR;

	if (!token)
		token = &s_dummy;

	/* Get gateway object */
	CURL* curl = curl_easy_init();
	if (curl) {
		/* Set gateway endpoint url */
		curl_easy_setopt(curl, CURLOPT_URL, DISCORD_API_GATEWAY_BOT);

		/* Set authorization header */
		char auth[80];
		sprintf(auth, "Authorization: Bot %.59s", token);
		curl_slist* headers = curl_slist_append(nullptr, auth);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* Set a callback to receive data */
		sReceiveData data { nullptr, 0 };
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

		/* Perform request */
		result = curl_easy_perform(curl);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		/* Parse received data into json */
		if (result == CURLE_OK)
			document.Parse(reinterpret_cast<const char*>(data.data), data.size);
		free(data.data);
	}

	return CURLE_OK;
}

cGateway::cGateway(const char* token) {
	CURLcode result = cDiscord::GetGateway(token, m_document);
	if (result == CURLE_OK) {
		if (m_document.HasMember("url")) {
			/* If url exists, parse as valid gateway response object */
			m_url    = m_document["url"].GetString();
			m_shards = m_document["shards"].GetInt();

			/* Parse session start limit object */
			rapidjson::Value& json_session_start_limit = m_document["session_start_limit"];
			m_session_start_limit.m_total           = json_session_start_limit["total"].GetInt();
			m_session_start_limit.m_remaining       = json_session_start_limit["remaining"].GetInt();
			m_session_start_limit.m_reset_after     = json_session_start_limit["reset_after"].GetInt();
			m_session_start_limit.m_max_concurrency = json_session_start_limit["max_concurrency"].GetInt();
		}
		else
			m_pError = new cJsonError(m_document);
		return;
	}
	m_pError = new cJsonError();
}
