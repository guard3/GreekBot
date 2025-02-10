cmake_minimum_required(VERSION 3.25)
project(GreekBot)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if(MSVC)
	add_definitions(/bigobj)
endif()

add_executable(GreekBot main.cpp Discord/Bot.cpp Discord/Bot.h Discord/Common.h Discord/Interaction/InteractionBase.cpp Discord/Interaction/InteractionBase.h Discord/Objects/Member.cpp Discord/Objects/Member.h Discord/Objects/User.cpp Discord/Objects/User.h Discord/Objects/Component.h Discord/Objects/Emoji.h Discord/Objects/Guild.h Discord/Objects/Channel.h Discord/Objects/Message.h Database/Database.h Database/Database.cpp Discord/Objects/Embed.h Discord/Objects/Embed/Embed.cpp Discord/Objects/Role.h Discord/Objects/Role.cpp GreekBot/GreekBot.h GreekBot/GreekBot.cpp GreekBot/GreekBot_Leaderboard.cpp GreekBot/GreekBot_Avatar.cpp GreekBot/GreekBot_Role.cpp GreekBot/GreekBot_Button.cpp Discord/Objects/Message.cpp Discord/Coroutines/Task.h Discord/Gateway/Gateway.h Discord/Gateway/Gateway.cpp Discord/Gateway/GatewayCommands.cpp Discord/Gateway/GatewayEvents.cpp Discord/Base/Color.cpp Discord/Objects/Guild.cpp Discord/Gateway/GatewayImpl.h Discord/Gateway/GatewayImpl.cpp Discord/Gateway/GatewayHttp.cpp Discord/Exception.h Discord/Exception.cpp Discord/Utils.h Discord/Utils/Utils.cpp GreekBot/GreekBot_Ban.cpp Discord/Objects/Channel.cpp Discord/Utils/Base64.cpp Discord/Objects/Embed/EmbedMedia.h Discord/Objects/Embed/EmbedAuthor.h Discord/Objects/Embed/EmbedFooter.h Discord/Objects/Emoji.cpp GreekBot/GreekBot_Clear.cpp Discord/Gateway/GuildMembersResult.h Discord/Gateway/GuildMembersResult.cpp Discord/Coroutines.h Discord/Coroutines/AsyncGenerator.h Discord/Coroutines/Generator.h Discord/Gateway/Application.h Discord/Gateway/Application.cpp Discord/Common/Ptr.h
	boost/impl.cpp
	GreekBot/GreekBot_Members.cpp
	Discord/Objects/Modal.h
	Discord/Objects/Modal.cpp
	Discord/Objects/Component/TextInput.h
	Discord/Objects/Component/Component.cpp
	Discord/Objects/Component/Button.h
	Discord/Objects/Component/SelectMenu.h
	Discord/Objects/Embed/EmbedField.h
	GreekBot/GreekBot_Starboard.cpp
	Discord/Objects/Attachment.h
	Discord/Objects/Attachment.cpp
	Discord/Interaction.h
	Discord/Interaction/AppCmdInteraction.h
	Discord/Interaction/MsgCompInteraction.h
	Discord/Interaction/AppCmdInteraction.cpp
	Discord/Interaction/MsgCompInteraction.cpp
	Discord/Interaction/ModalSubmitInteraction.h
	Discord/Interaction/ModalSubmitInteraction.cpp
	GreekBot/GreekBot_Timestamp.cpp
	Discord/CDN.h
	Discord/CDN.cpp
	Discord/Bot_Interaction.cpp
	Discord/InteractionFwd.h
	GreekBot/GreekBot_MessageEvents.cpp
	Discord/Objects/Component/SelectMenu.cpp
	Discord/Objects/Component/Button.cpp
	Discord/Objects/Component/TextInput.cpp
	Discord/Objects/Component/UnsupportedComponent.h
	Discord/Objects/Component/UnsupportedComponent.cpp
	Discord/Objects/MessageFwd.h
	Discord/Objects/EmojiFwd.h
	Discord/Gateway/GatewayGuildMembers.cpp
	Discord/VoiceState.h
	Discord/VoiceState.cpp
	GreekBot/GreekBot_Voice.cpp
	Discord/Bot_ChannelMessages.cpp
	GreekBot/GreekBot_TimedActions.cpp
	Discord/Common/Visitor.h
	Discord/Base.h
	Discord/Base/Snowflake.cpp
	Discord/Base/SnowflakeFwd.h
	Discord/BaseFwd.h
	Discord/Base/Color.h
	Discord/Base/ColorFwd.h
	Discord/Objects/RoleFwd.h
	Discord/Objects/UserFwd.h
	Discord/Objects/ChannelFwd.h
	Discord/Objects/EmbedFwd.h
	Discord/Objects/GuildFwd.h
	Discord/Objects/MemberFwd.h
	Discord/Common/Concepts.h
	Discord/VoiceStateFwd.h
	Discord/Gateway/GatewayIntents.h
	Database/SQLite.h
	Database/SQLite.cpp
	GreekBot/LearningGreek.h
	GreekBot/GreekBot_Database.cpp
	Database/Leaderboard/Leaderboard.h
	Database/Leaderboard/Leaderboard.cpp
	Database/DBLeaderboard.h
		GreekBot/GreekBot_Infractions.cpp
		Database/Infractions/Infractions.h
		Database/Infractions/Infractions.cpp
		Database/DBInfractions.h
)

target_include_directories(
    GreekBot PRIVATE
    ${PROJECT_SOURCE_DIR}/Discord
    ${PROJECT_SOURCE_DIR}/Discord/Objects
    ${PROJECT_SOURCE_DIR}/Discord/Gateway
    ${PROJECT_SOURCE_DIR}/GreekBot
    ${PROJECT_SOURCE_DIR}/Database
)

target_compile_definitions(GreekBot PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# Find packages
find_package(Boost 1.86.0 REQUIRED COMPONENTS url OPTIONAL_COMPONENTS json) # Boost/JSON and Boost/URL are required, although Boost/JSON can be used as header-only
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 3.35.0 REQUIRED)

# Configure compilation of header-only Boost libraries
target_compile_definitions(GreekBot PRIVATE BOOST_ASIO_SEPARATE_COMPILATION BOOST_BEAST_SEPARATE_COMPILATION)
if(NOT Boost_JSON_FOUND)
    target_compile_definitions(GreekBot PRIVATE BOOST_JSON_SEPARATE_COMPILATION)
endif()

# Include Libraries
target_include_directories(GreekBot SYSTEM PRIVATE
	${OPENSSL_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	${SQLite3_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(GreekBot
	OpenSSL::SSL
	${Boost_LIBRARIES}
	SQLite::SQLite3
	ZLIB::ZLIB
)

# Prepare SQL query defines
function(define_query VAR_NAME FILE_NAME)
    file(READ ${FILE_NAME} CONTENT)
    string(REGEX REPLACE "[\n\t ]+" " " CONTENT "R\"(${CONTENT})\"")
    set(${VAR_NAME} ${CONTENT} PARENT_SCOPE)
endfunction()

define_query(QUERY_LB_UPDATE  Database/Leaderboard/query_lb_update.sql)
define_query(QUERY_LB_RANK    Database/Leaderboard/query_lb_rank.sql  )
define_query(QUERY_LB_TOP     Database/Leaderboard/query_lb_top.sql   )
configure_file(Database/Leaderboard/LeaderboardQueries.h.in LeaderboardQueries.h)

define_query(QUERY_WARN_REGISTER Database/Infractions/query_warn_register.sql)
define_query(QUERY_WARN_GET      Database/Infractions/query_warn_get.sql)
configure_file(Database/Infractions/InfractionsQueries.h.in InfractionsQueries.h)

define_query(QUERY_INIT       Database/query_init.sql      )
define_query(QUERY_WC_REG_MBR Database/query_wc_reg_mbr.sql)
define_query(QUERY_WC_UPD_MSG Database/query_wc_upd_msg.sql)
define_query(QUERY_WC_GET_MSG Database/query_wc_get_msg.sql)
define_query(QUERY_WC_DEL_MBR Database/query_wc_del_mbr.sql)
define_query(QUERY_WC_EDT_MSG Database/query_wc_edt_msg.sql)

define_query(QUERY_SB_REGISTER_REACTION Database/query_sb_register_reaction.sql)
define_query(QUERY_SB_REMOVE_REACTION Database/query_sb_remove_reaction.sql)
define_query(QUERY_SB_REMOVE_ALL Database/query_sb_remove_all.sql)
define_query(QUERY_SB_REGISTER_MESSAGE Database/query_sb_register_msg.sql)
define_query(QUERY_SB_REMOVE_MESSAGE Database/query_sb_remove_msg.sql)
define_query(QUERY_SB_GET_MESSAGE_AUTHOR Database/query_sb_get_msg_author.sql)
define_query(QUERY_SB_GET_TOP_10 Database/query_sb_get_top_10.sql)
define_query(QUERY_SB_GET_RANK Database/query_sb_get_rank.sql)

define_query(QUERY_REGISTER_MESSAGE Database/query_register_message.sql)
define_query(QUERY_GET_MESSAGE Database/query_get_message.sql)
define_query(QUERY_DELETE_MESSAGE Database/query_delete_message.sql)
define_query(QUERY_CLEANUP_MESSAGES Database/query_cleanup_messages.sql)

define_query(QUERY_REGISTER_TEMPORARY_BAN Database/query_register_temporary_ban.sql)
define_query(QUERY_REMOVE_TEMPORARY_BAN   Database/query_remove_temporary_ban.sql  )
configure_file(Database/Queries.h.in Queries.h)

# Add the binary directory to the include directories, since that's where the header files are generated
target_include_directories(GreekBot PRIVATE ${CMAKE_CURRENT_BINARY_DIR})