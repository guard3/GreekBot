#ifndef DISCORD_COMPONENTTYPE_H
#define DISCORD_COMPONENTTYPE_H
#include "Base.h"

enum eComponentType {
	COMPONENT_ACTION_ROW = 1,    // Container to display a row of interactive components
	COMPONENT_BUTTON,            // Button object
	COMPONENT_SELECT_MENU,       // Select menu for picking from defined text options
	COMPONENT_TEXT_INPUT,        // Text input object
	COMPONENT_TEXT_DISPLAY = 10, // Markdown text
	COMPONENT_LABEL = 18         // Container associating a label and description with a component
};

eComponentType
tag_invoke(boost::json::value_to_tag<eComponentType>, const boost::json::value& v);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, eComponentType);
#endif //DISCORD_COMPONENTTYPE_H