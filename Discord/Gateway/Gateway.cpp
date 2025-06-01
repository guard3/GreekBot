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
cGateway::ResumeOnEventStrand() { co_await m_pImpl->ResumeOnEventStrand(); }
cTask<>
cGateway::WaitOnEventStrand(std::chrono::milliseconds d) { co_await m_pImpl->WaitOnEventStrand(d); }
/* ================================================================================================================== */
cAsyncGenerator<cMember>
cGateway::RequestGuildMembers(const cSnowflake& guild_id, std::string_view query) {
	return m_pImpl->RequestGuildMembers(guild_id, query);
}
cAsyncGenerator<cMember>
cGateway::RequestGuildMembers(const cSnowflake& guild_id, std::span<const cSnowflake> user_ids) {
	return m_pImpl->RequestGuildMembers(guild_id, user_ids);
}
/* ================================================================================================================== */
std::string_view
cGateway::GetToken() const noexcept { return m_pImpl->GetToken(); }
void
cGateway::Run() { m_pImpl->Run(); }