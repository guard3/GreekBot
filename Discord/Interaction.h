#ifndef GREEKBOT_INTERACTION_H
#define GREEKBOT_INTERACTION_H
#include "Interaction/AppCmdInteraction.h"
#include "Interaction/MsgCompInteraction.h"
#include "Interaction/ModalSubmitInteraction.h"
#include <functional>

template<iInteractionVisitor Visitor>
inline decltype(auto) cInteraction::Visit(Visitor&& visitor) {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<cAppCmdInteraction*>(this));
		case INTERACTION_MESSAGE_COMPONENT:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<cMsgCompInteraction*>(this));
		default:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<cModalSubmitInteraction*>(this));
	}
}
template<iInteractionVisitor Visitor>
inline decltype(auto) cInteraction::Visit(Visitor&& visitor) const {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<const cAppCmdInteraction*>(this));
		case INTERACTION_MESSAGE_COMPONENT:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<const cMsgCompInteraction*>(this));
		default:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<const cModalSubmitInteraction*>(this));
	}
}
template<typename R, iInteractionVisitorR<R> Visitor>
inline R cInteraction::Visit(Visitor&& visitor) {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<cAppCmdInteraction*>(this)));
		case INTERACTION_MESSAGE_COMPONENT:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<cMsgCompInteraction*>(this)));
		default:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<cModalSubmitInteraction*>(this)));
	}
}
template<typename R, iInteractionVisitorR<R> Visitor>
inline R cInteraction::Visit(Visitor&& visitor) const {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<const cAppCmdInteraction*>(this)));
		case INTERACTION_MESSAGE_COMPONENT:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<const cMsgCompInteraction*>(this)));
		default:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<const cModalSubmitInteraction*>(this)));
	}
}
#endif /* GREEKBOT_INTERACTION_H */
