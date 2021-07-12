#include <rapidjson/document.h>

enum eJsonErrorCode {
	DISCORD_JSON_UNKNOWN_ERROR = -1,
	DISCORD_JSON_GENERAL_ERROR
};

class cJsonError final {
private:
	const char     *m_message;
	eJsonErrorCode  m_code;

public:
	cJsonError() : m_message("Unknown Error"), m_code(DISCORD_JSON_UNKNOWN_ERROR) {}
	cJsonError(const rapidjson::Document& document) : m_message(document["message"].GetString()), m_code(static_cast<eJsonErrorCode>(document["code"].GetInt())) {}
	cJsonError(const cJsonError& ) = delete;
	cJsonError(      cJsonError&&) = delete;

	const char* GetMessage() const { return m_message; }
	eJsonErrorCode GetCode() const { return m_code;    }
};