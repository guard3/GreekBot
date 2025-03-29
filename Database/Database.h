#ifndef GREEKBOT_DATABASE_H
#define GREEKBOT_DATABASE_H
#include "Coroutines.h"
#include "Message.h"
#include "Transaction.h"
#include <vector>
/* ========== Make cSnowflake bindable to prepared statements as an int ============================================= */
template<>
struct sqlite::binder<cSnowflake> {
	static void bind(statement_ref stmt, int index, const cSnowflake& id, std::error_code& ec) noexcept {
		stmt.bind(index, id.ToInt(), ec);
	}
};
/* ================================================================================================================== */
class cDatabase final {
public:
	cDatabase() = delete;
	cDatabase(const cDatabase&) = delete;

	cDatabase& operator=(const cDatabase&) = delete;

	static void Initialize() noexcept;

	/* Expose some internals while refactoring takes place - TODO: remove or make private */
	static sqlite::connection CreateInstance();
	static cTask<> ResumeOnDatabaseStrand();
	static cTask<> Wait(std::chrono::milliseconds);

	[[nodiscard]] static cTask<cTransaction> BorrowDatabase();
	[[nodiscard]] static cTask<> ReturnDatabase(cTransaction);

	static cTask<uint64_t> WC_RegisterMember(const cMember&);
	static cTask<> WC_UpdateMessage(const cUser&, const cMessage&);
	static cTask<int64_t> WC_GetMessage(const cMemberUpdate&);
	static cTask<> WC_EditMessage(int64_t);
	static cTask<uint64_t> WC_DeleteMember(const cUser&);

	static cTask<> RegisterTemporaryBan(crefUser user, std::chrono::sys_days expires_at);
	static cTask<std::vector<cSnowflake>> GetExpiredTemporaryBans();
	static cTask<> RemoveTemporaryBan(crefUser user);
	static cTask<> RemoveTemporaryBans(std::span<const cSnowflake> user_ids);
};
#endif /* GREEKBOT_DATABASE_H */