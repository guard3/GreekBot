#pragma once
#ifndef _GREEKBOT_SHASHCOMMAND_H_
#define _GREEKBOT_SHASHCOMMAND_H_

enum eCommandOptionType {
	COMMAND_OPTION_SUB_COMMAND = 1,
	COMMAND_OPTION_SUB_COMMAND_GROUP,
	COMMAND_OPTION_STRING,
	COMMAND_OPTION_INTEGER,
	COMMAND_OPTION_BOOLEAN,
	COMMAND_OPTION_USER,
	COMMAND_OPTION_CHANNEL,
	COMMAND_OPTION_ROLE,
	COMMAND_OPTION_MENTIONABLE
};

class cSlashCommand final {
private:
	char m_name[32];
	char m_description[100];
	
public:
	cSlashCommand(const char* name, const char* description);
	
	const char* GetName()        const { return m_name;        }
	const char* GetDescription() const { return m_description; }
};

class cSlashCommandBuilder final {
private:
	char m_name[32];
};


#endif /* _GREEKBOT_SLASHCOMMAND_H_ */
