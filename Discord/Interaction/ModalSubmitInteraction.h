#ifndef GREEKBOT_MODALSUBMITINTERACTION_H
#define GREEKBOT_MODALSUBMITINTERACTION_H
#include "InteractionBase.h"

class cModalSubmitInteraction final : public cInteraction {
private:
	std::string m_custom_id;
	std::vector<cModalSubmitData> m_submit; // TODO: Use component objects

public:
	explicit cModalSubmitInteraction(const json::object&, const json::object&);

	std::string_view GetCustomId() const noexcept { return m_custom_id; }
	std::span<const cModalSubmitData> GetSubmittedData() const noexcept { return m_submit; }
};
#endif /* GREEKBOT_MODALSUBMITINTERACTION_H */
