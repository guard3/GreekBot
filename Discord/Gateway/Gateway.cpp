#include "GatewayImpl.h"
#include "json.h"
/* ================================================================================================================== */
cGateway::cGateway(const char* t, eIntent i) : m_pImpl(cHandle::MakeUnique<implementation>(this, t, i)) {}
cGateway::~cGateway() = default;
/* ================================================================================================================== */
cTask<json::value>
cGateway::DiscordGet(const std::string& t, std::initializer_list<cHttpField> f) {
	return m_pImpl->DiscordRequest(beast::http::verb::get, t, nullptr, f);
}
cTask<json::value>
cGateway::DiscordPost(const std::string& t, const json::object& obj, std::initializer_list<cHttpField> f) {
	return m_pImpl->DiscordRequest(beast::http::verb::post, t, &obj, f);
}
cTask<json::value>
cGateway::DiscordPatch(const std::string& t, const json::object& obj, std::initializer_list<cHttpField> f) {
	return m_pImpl->DiscordRequest(beast::http::verb::patch, t, &obj, f);
}
cTask<json::value>
cGateway::DiscordPut(const std::string& t, std::initializer_list<cHttpField> f) {
	return m_pImpl->DiscordRequest(beast::http::verb::put, t, nullptr, f);
}
cTask<json::value>
cGateway::DiscordPut(const std::string& t, const json::object& obj, std::initializer_list<cHttpField> f) {
	return m_pImpl->DiscordRequest(beast::http::verb::put, t, &obj, f);
}
cTask<json::value>
cGateway::DiscordDelete(const std::string& t, std::initializer_list<cHttpField> f) {
	return m_pImpl->DiscordRequest(beast::http::verb::delete_, t, nullptr, f);
}
/* ================================================================================================================== */
cTask<>
cGateway::ResumeOnEventThread() { return m_pImpl->ResumeOnEventThread(); }
/* ================================================================================================================== */
const char*
cGateway::GetHttpAuthorization() const noexcept { return m_pImpl->GetHttpAuthorization(); }
const char*
cGateway::GetToken() const noexcept { return m_pImpl->GetToken(); }
void
cGateway::Run() { m_pImpl->Run(); }