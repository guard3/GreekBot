#ifndef GREEKBOT_NICKNAMES_H
#define GREEKBOT_NICKNAMES_H
#include "Database.h"
#include "Member.h"
#include "MessageFwd.h"
#include <optional>

struct nickname_entry {
	std::optional<cSnowflake> msg_id;
	std::string nick;
};

class cNicknamesDAO : public cBaseDAO {
public:
	explicit cNicknamesDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	/**
	 * Retrieves a nickname entry from the database
	 * @param user The id of the user
	 * @return An awaitable returning a nickname entry, containing an (optional) notification message id and a (possibly empty) nickname string
	 */
	[[nodiscard]]
	cTask<nickname_entry> Get(crefUser user);

	/**
	 * Clears the saved notification message id for a particular user
	 * @param user The id of the user
	 * @return An awaitable returning the (optional) notification message id prior to the change
	 */
	[[nodiscard]]
	cTask<std::optional<cSnowflake>> DeleteMessage(crefUser user);

	/**
	 * Updates the saved nickname for a particular user
	 * @param user The id of the user
	 * @param nick The new nickname, which should be non-empty
	 * @return A void awaitable
	 */
	[[nodiscard]]
	cTask<> Update(crefUser user, std::string_view nick);

	/**
	 * Updates the saved notification message if for a particular user
	 * @param user The id of the user
	 * @param msg The id of the notification message
	 * @return A void awaitable
	 */
	[[nodiscard]]
	cTask<> RegisterMessage(crefUser user, crefMessage msg);
};

#endif //GREEKBOT_NICKNAMES_H
