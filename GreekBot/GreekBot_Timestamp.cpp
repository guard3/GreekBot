#include "GreekBot.h"
#include "Utils.h"

cTask<>
cGreekBot::process_timestamp(cAppCmdInteraction& i) {
	using namespace std::chrono;
	/* Variables */
	cMessageParams msg;
	std::string_view input;
	std::string_view style = "f";
	try {
		auto& subcmd = i.GetOptions().front();
		switch (cUtils::CRC32(0, subcmd.GetName())) {
			case 0x08875CAC: // "help"
				msg = cMessageParams{
					kw::flags=MESSAGE_FLAG_EPHEMERAL,
					kw::content="Use this command to format an **ISO 8601** timestamp to a Discord format for messages. The accepted string specification is described as follows:\n"
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
					            "All of the above describe <t:1703507760:f>"
				};
				break;
			case 0xDEBA72DF: // "format"
				/* Parse subcommand options */
				for (auto& opt : subcmd.GetOptions())
					(opt.GetName() == "style" ? style : input) = opt.GetValue<APP_CMD_OPT_STRING>();
				/* Create message */
				msg = cMessageParams{
					kw::content=fmt::format("<t:{0}:{1}>\n```<t:{0}:{1}>```", floor<seconds>(cUtils::ParseISOTimestamp(input)).time_since_epoch().count(), style),
					kw::components={
						cActionRow{
							cButton{
								BUTTON_STYLE_SECONDARY,
								fmt::format("DLT#{}", i.GetUser().GetId()),
								kw::label="Dismiss"
							}
						}
					}
				};
				break;
			default:
				throw std::runtime_error("meow");
		}
	} catch (const std::invalid_argument&) {
		/* The input timestamp string was invalid */
		msg = cMessageParams{
			kw::flags=MESSAGE_FLAG_EPHEMERAL,
			kw::content=fmt::format("`{:<1}` is not a valid timestamp 💀 Try again!", input)
		};
	} catch (...) {
		/* Another error occurred */
		report_error("process_timestamp", std::current_exception());
		msg = cMessageParams{
			kw::flags=MESSAGE_FLAG_EPHEMERAL,
			kw::content="An unexpected error has occurred. Try again later."
		};
	}
	co_await InteractionSendMessage(i, msg);
}