#include "SlashCommand.h"
#include <cstring>

cSlashCommand::cSlashCommand(const char* name, const char* description) {
	if (name) {
		strncpy(m_name, name, 31);
		m_name[31] = '\0';
	}
	else *m_name = '\0';
	
	if (description) {
		strncpy(m_description, description, 99);
		m_description[99] = '\0';
	}
	else *m_description = '\0';
}
