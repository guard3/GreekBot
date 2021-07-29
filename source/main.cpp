#include "Bot.h"
#include "Types.h"

int main() {
	
	cSnowflake snow("350234668680871946");
	printf("%u\n", snow.GetTimestamp());
	
	cBot g("ODYzNDk5MTUxMTgxMjE3Nzky.YOnyQA.6XWkgM_P4S4lE-t8jhT_dD2qQlk");
	g.Run();
	
	return 0;
}
