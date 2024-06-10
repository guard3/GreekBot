#include "GatewayImpl.h"
/* ================================================================================================================== */
cGateway::cGateway(std::string_view t, eIntent i) : m_pImpl(std::make_unique<implementation>(this, t, i)) {}
cGateway::~cGateway() = default;
/* ================================================================================================================== */
cTask<json::value>
cGateway::DiscordGet(std::string_view t, std::span<const cHttpField> f) {
	return m_pImpl->DiscordGet(t, f);
}
cTask<json::value>
cGateway::DiscordPost(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return m_pImpl->DiscordPost(t, o, f);
}
cTask<json::value>
cGateway::DiscordPatch(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return m_pImpl->DiscordPatch(t, o, f);
}
cTask<json::value>
cGateway::DiscordPut(std::string_view t, std::span<const cHttpField> f) {
	return m_pImpl->DiscordPut(t, f);
}
cTask<json::value>
cGateway::DiscordPut(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return m_pImpl->DiscordPut(t, o, f);
}
cTask<json::value>
cGateway::DiscordDelete(std::string_view t, std::span<const cHttpField> f) {
	return m_pImpl->DiscordDelete(t, f);
}
/* ================================================================================================================== */
cTask<json::value>
cGateway::DiscordPostNoRetry(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return m_pImpl->DiscordPostNoRetry(t, o, f);
}
/* ================================================================================================================== */
cTask<>
cGateway::ResumeOnEventThread() { co_await m_pImpl->ResumeOnEventThread(); }
cTask<>
cGateway::WaitOnEventThread(chrono::milliseconds d) { co_await m_pImpl->WaitOnEventThread(d); }
cAsyncGenerator<cMember>
cGateway::get_guild_members(const cSnowflake& g, const std::string& s, const std::vector<cSnowflake>& u) {
	return m_pImpl->get_guild_members(g, s, u);
}
/* ================================================================================================================== */
std::string_view
cGateway::GetHttpAuthorization() const noexcept { return m_pImpl->GetHttpAuthorization(); }
std::string_view
cGateway::GetToken() const noexcept { return m_pImpl->GetToken(); }
void
cGateway::Run() { m_pImpl->Run(); }