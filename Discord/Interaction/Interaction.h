#ifndef GREEKBOT_INTERACTION_H
#define GREEKBOT_INTERACTION_H
#include "ApplicationCommandInteraction.h"
#include "MessageComponentInteraction.h"
#include "ModalSubmitInteraction.h"

template<iInteractionVisitor Visitor>
inline decltype(auto) cInteraction::Visit(Visitor&& visitor) {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<cApplicationCommandInteraction*>(this));
		case INTERACTION_MESSAGE_COMPONENT:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<cMessageComponentInteraction*>(this));
		case INTERACTION_MODAL_SUBMIT:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<cModalSubmitInteraction*>(this));
		default:
			throw std::runtime_error("skata");
	}
}
template<iInteractionVisitor Visitor>
inline decltype(auto) cInteraction::Visit(Visitor&& visitor) const {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<const cApplicationCommandInteraction*>(this));
		case INTERACTION_MESSAGE_COMPONENT:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<const cMessageComponentInteraction*>(this));
		case INTERACTION_MODAL_SUBMIT:
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<const cModalSubmitInteraction*>(this));
		default:
			throw std::runtime_error("skata");
	}
}
template<typename R, iInteractionVisitorR<R> Visitor>
inline R cInteraction::Visit(Visitor&& visitor) {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<cApplicationCommandInteraction*>(this)));
		case INTERACTION_MESSAGE_COMPONENT:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<cMessageComponentInteraction*>(this)));
		case INTERACTION_MODAL_SUBMIT:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<cModalSubmitInteraction*>(this)));
		default:
			throw std::runtime_error("skata");
	}
}
template<typename R, iInteractionVisitorR<R> Visitor>
inline R cInteraction::Visit(Visitor&& visitor) const {
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<const cApplicationCommandInteraction*>(this)));
		case INTERACTION_MESSAGE_COMPONENT:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<const cMessageComponentInteraction*>(this)));
		case INTERACTION_MODAL_SUBMIT:
			return static_cast<R>(std::invoke(std::forward<Visitor>(visitor), *static_cast<const cModalSubmitInteraction*>(this)));
		default:
			throw std::runtime_error("skata");
	}
}
#endif //GREEKBOT_INTERACTION_H
