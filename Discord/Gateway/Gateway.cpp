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
cGateway::RequestGuildMembers(const cSnowflake& guild_id) {
	return m_pImpl->RequestGuildMembers(guild_id);
}
cAsyncGenerator<cMember>
cGateway::RequestGuildMembers(const cSnowflake& guild_id, const cRequestGuildMembers& rgm) {
	return m_pImpl->RequestGuildMembers(guild_id, rgm);
}
/* ================================================================================================================== */
std::string_view
cGateway::GetHttpAuthorization() const noexcept { return m_pImpl->GetHttpAuthorization(); }
std::string_view
cGateway::GetToken() const noexcept { return m_pImpl->GetToken(); }
void
cGateway::Run() { m_pImpl->Run(); }