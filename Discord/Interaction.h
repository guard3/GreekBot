#ifndef DISCORD_INTERACTION_H
#define DISCORD_INTERACTION_H
#include "Interaction/AppCmdInteraction.h"
#include "Interaction/MsgCompInteraction.h"
#include "Interaction/ModalSubmitInteraction.h"

template<iInteractionVisitor Visitor>
inline decltype(auto) cInteraction::Visit(Visitor&& visitor) {
	using namespace detail;
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return std::forward<Visitor>(visitor)(*static_cast<cAppCmdInteraction*>(this));
		case INTERACTION_MESSAGE_COMPONENT:
			return std::forward<Visitor>(visitor)(*static_cast<cMsgCompInteraction*>(this));
		default:
			return std::forward<Visitor>(visitor)(*static_cast<cModalSubmitInteraction*>(this));
	}
}
template<iInteractionVisitor Visitor>
inline decltype(auto) cInteraction::Visit(Visitor&& visitor) const {
	using namespace detail;
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return std::forward<Visitor>(visitor)(*static_cast<const cAppCmdInteraction*>(this));
		case INTERACTION_MESSAGE_COMPONENT:
			return std::forward<Visitor>(visitor)(*static_cast<const cMsgCompInteraction*>(this));
		default:
			return std::forward<Visitor>(visitor)(*static_cast<const cModalSubmitInteraction*>(this));
	}
}
template<typename R, iInteractionVisitorR<R> Visitor>
inline R cInteraction::Visit(Visitor&& visitor) {
	using namespace detail;
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return static_cast<R>(std::forward<Visitor>(visitor)(*static_cast<cAppCmdInteraction*>(this)));
		case INTERACTION_MESSAGE_COMPONENT:
			return static_cast<R>(std::forward<Visitor>(visitor)(*static_cast<cMsgCompInteraction*>(this)));
		default:
			return static_cast<R>(std::forward<Visitor>(visitor)(*static_cast<cModalSubmitInteraction*>(this)));
	}
}
template<typename R, iInteractionVisitorR<R> Visitor>
inline R cInteraction::Visit(Visitor&& visitor) const {
	using namespace detail;
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return static_cast<R>(std::forward<Visitor>(visitor)(*static_cast<const cAppCmdInteraction*>(this)));
		case INTERACTION_MESSAGE_COMPONENT:
			return static_cast<R>(std::forward<Visitor>(visitor)(*static_cast<const cMsgCompInteraction*>(this)));
		default:
			return static_cast<R>(std::forward<Visitor>(visitor)(*static_cast<const cModalSubmitInteraction*>(this)));
	}
}
#endif /* DISCORD_INTERACTION_H */