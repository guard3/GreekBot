#include "GatewayImpl.h"
#include "json.h"
/* ================================================================================================================== */
cGateway::cGateway(const char* t, eIntent i) : m_pImpl(cHandle::MakeUnique<implementation>(this, t, i)) {}
cGateway::~cGateway() = default;
/* ================================================================================================================== */
cTask<json::value>
cGateway::DiscordGet(const std::string& t, const tHttpFields& f) {
	return m_pImpl->DiscordRequest(beast::http::verb::get, t, nullptr, f);
}
cTask<json::value>
cGateway::DiscordPost(const std::string& t, const json::object& o, const tHttpFields& f) {
	return m_pImpl->DiscordRequest(beast::http::verb::post, t, &o, f);
}
cTask<json::value>
cGateway::DiscordPatch(const std::string& t, const json::object& o, const tHttpFields& f) {
	return m_pImpl->DiscordRequest(beast::http::verb::patch, t, &o, f);
}
cTask<json::value>
cGateway::DiscordPut(const std::string& t, const tHttpFields& f) {
	return m_pImpl->DiscordRequest(beast::http::verb::put, t, nullptr, f);
}
cTask<json::value>
cGateway::DiscordPut(const std::string& t, const json::object& o, const tHttpFields& f) {
	return m_pImpl->DiscordRequest(beast::http::verb::put, t, &o, f);
}
cTask<json::value>
cGateway::DiscordDelete(const std::string& t, const tHttpFields& f) {
	return m_pImpl->DiscordRequest(beast::http::verb::delete_, t, nullptr, f);
}
/* ================================================================================================================== */
cTask<json::value>
cGateway::DiscordPostNoRetry(const std::string& t, const json::object& o, const tHttpFields& f) {
	return m_pImpl->DiscordRequestNoRetry(beast::http::verb::post, t, &o, f);
}
/* ================================================================================================================== */
cTask<>
cGateway::ResumeOnEventThread() { return m_pImpl->ResumeOnEventThread(); }
cTask<>
cGateway::WaitOnEventThread(chrono::milliseconds d) { return m_pImpl->WaitOnEventThread(d); }
/* ================================================================================================================== */
const char*
cGateway::GetHttpAuthorization() const noexcept { return m_pImpl->GetHttpAuthorization(); }
const char*
cGateway::GetToken() const noexcept { return m_pImpl->GetToken(); }
void
cGateway::Run() { m_pImpl->Run(); }