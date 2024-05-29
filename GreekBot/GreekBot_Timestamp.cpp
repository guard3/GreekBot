#include "GreekBot.h"
#include "Utils.h"

cTask<>
cGreekBot::process_timestamp(cAppCmdInteraction& i) HANDLER_BEGIN {
	using namespace std::chrono;
	/* Variables */
	cMessageParams response;
	std::string_view input;
	std::string_view style = "f";
	switch (auto& subcmd = i.GetOptions().front(); cUtils::CRC32(0, subcmd.GetName())) {
		default:
			throw std::runtime_error("Unknown subcommand");
		case 0x08875CAC: // "help"
			response
				.SetFlags(MESSAGE_FLAG_EPHEMERAL)
				.SetContent("Use this command to format an **ISO 8601** timestamp to a Discord format for messages. The accepted string specification is described as follows:\n"
				            "## Basic format\n"
				            "- `YYYYMMDDThhmmss     `\n"
				            "- `YYYYMMDDThhmmssZ    ` (UTC time)\n"
				            "- `YYYYMMDDThhmmss±hh  ` (Timezone offset)\n"
				            "- `YYYYMMDDThhmmss±hhmm` (Timezone offset)\n"
				            "## Extended format\n"
				            "- `YYYY-MM-DDThh:mm:ss      `\n"
				            "- `YYYY-MM-DDThh:mm:ssZ     ` (UTC time)\n"
				            "- `YYYY-MM-DDThh:mm:ss±hh:mm` (Timezone offset)\n"
				            "## Examples\n"
				            "- `20231225T123600`\n"
				            "- `20231225T103600-02`\n"
				            "- `2023-12-25T15:36:00+03:00`\n"
				            "All of the above describe <t:1703507760:f>");
			break;
		case 0xDEBA72DF: // "format"
			/* Parse subcommand options */
			for (auto& opt : subcmd.GetOptions())
				(opt.GetName() == "style" ? style : input) = opt.GetValue<APP_CMD_OPT_STRING>();
			try {
				/* Create message */
				response
					.SetContent(fmt::format("<t:{0}:{1}>\n```<t:{0}:{1}>```", floor<seconds>(cUtils::ParseISOTimestamp(input)).time_since_epoch().count(), style))
					.SetComponents({
						cActionRow{
							cButton{
								BUTTON_STYLE_SECONDARY,
								fmt::format("DLT#{}", i.GetUser().GetId()),
								"Dismiss"
							}
						}
					});
			} catch (const std::invalid_argument&) {
				/* The input timestamp string was invalid */
				response
					.SetFlags(MESSAGE_FLAG_EPHEMERAL)
					.SetContent(fmt::format("`{:<1}` is not a valid timestamp 💀 Try again!", input));
			}
			break;
	}
	co_await InteractionSendMessage(i, response);
} HANDLER_END