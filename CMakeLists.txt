cmake_minimum_required(VERSION 3.17)
project(GreekBot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if (MSVC)
    add_definitions(/bigobj)
endif()

add_executable(GreekBot main.cpp Discord/Bot.cpp Discord/Bot.h Discord/Common/Common.h Discord/Interaction/InteractionBase.cpp Discord/Interaction/InteractionBase.h Discord/Objects/Member.cpp Discord/Objects/Member.h Discord/Objects/User.cpp Discord/Objects/User.h Discord/Objects/Component.h Discord/Objects/Emoji.h Discord/Objects/Guild.h Discord/Objects/Channel.h Discord/Objects/Message.h Database/Database.h Database/Database.cpp Discord/Objects/Embed.h Discord/Objects/Embed/Embed.cpp Discord/Objects/Role.h Discord/Objects/Role.cpp GreekBot/GreekBot.h GreekBot/GreekBot.cpp GreekBot/GreekBot_Leaderboard.cpp GreekBot/GreekBot_Avatar.cpp GreekBot/GreekBot_Role.cpp GreekBot/GreekBot_Button.cpp Discord/Objects/Message.cpp Discord/Coroutines/Task.h Discord/Gateway/Gateway.h Discord/Gateway/Gateway.cpp Discord/Gateway/GatewayCommands.cpp Discord/Gateway/GatewayEvents.cpp Discord/Common/Common.cpp Discord/Objects/Guild.cpp boost/beast.h Discord/Gateway/GatewayImpl.h Discord/Gateway/GatewayImpl.cpp Discord/Gateway/GatewayHttp.cpp GreekBot/GreekBot_Prune.cpp Discord/Exception.h Discord/Exception.cpp Discord/Common/Utils.h Discord/Common/Utils.cpp GreekBot/GreekBot_Ban.cpp Discord/Objects/Channel.cpp Discord/Common/Base64.cpp Discord/Objects/Embed/EmbedMedia.h Discord/Objects/Embed/EmbedAuthor.h Discord/Objects/Embed/EmbedFooter.h Discord/Objects/Emoji.cpp GreekBot/GreekBot_Dismiss.cpp Discord/Gateway/GuildMembersResult.h Discord/Gateway/GuildMembersResult.cpp Discord/Coroutines.h Discord/Coroutines/AsyncGenerator.h Discord/Coroutines/Generator.h Discord/Gateway/Application.h Discord/Gateway/Application.cpp Discord/Common/Ptr.h
        boost/impl.cpp
        GreekBot/GreekBot_Members.cpp
        Discord/Objects/Modal.h
        Discord/Objects/Modal.cpp
        Discord/kwarg.h
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
        Discord/Objects/EmojiFwd.h)

target_include_directories(
    GreekBot PRIVATE
    ${PROJECT_SOURCE_DIR}/boost
    ${PROJECT_SOURCE_DIR}/Discord
    ${PROJECT_SOURCE_DIR}/Discord/Common
    ${PROJECT_SOURCE_DIR}/Discord/Objects
    ${PROJECT_SOURCE_DIR}/Discord/Gateway
    ${PROJECT_SOURCE_DIR}/GreekBot
    ${PROJECT_SOURCE_DIR}/Database
)

option(GW_LOG_LVL "Whether or not to print logs from the gateway" 1)
if(GW_LOG_LVL EQUAL 1)
    target_compile_definitions(GreekBot PRIVATE GW_LOG_LVL_1)
elseif(GW_LOG_LVL EQUAL 2)
    target_compile_definitions(GreekBot PRIVATE GW_LOG_LVL_2)
    target_compile_definitions(GreekBot PRIVATE GW_LOG_LVL_1)
endif()

# Link OpenSSL
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
target_link_libraries(GreekBot OpenSSL::SSL)

# Link Boost
add_compile_definitions(BOOST_ASIO_SEPARATE_COMPILATION BOOST_BEAST_SEPARATE_COMPILATION)
set(Boost_USE_STATIC_LIBS     ON)
set(Boost_USE_MULTITHREADED   ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.84.0 REQUIRED OPTIONAL_COMPONENTS json)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
if(Boost_JSON_FOUND)
    target_link_libraries(GreekBot ${Boost_LIBRARIES})
else()
    target_sources(GreekBot PRIVATE boost/json.cpp)
endif()

# Link SQLite
find_package(SQLite3 3.35.0 REQUIRED)
include_directories(SYSTEM ${SQLite3_INCLUDE_DIRS})
target_link_libraries(GreekBot SQLite::SQLite3)

# Link ZLIB
find_package(ZLIB REQUIRED)
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
target_link_libraries(GreekBot ZLIB::ZLIB)

# Link {fmt} (since Clang 15 doesn't support C++20 <format>)
find_package(fmt 10.0.0 REQUIRED)
target_link_libraries(GreekBot fmt::fmt)

# Prepare SQL query defines
function(define_query VAR_NAME FILE_NAME)
    file(READ ${FILE_NAME} CONTENT)
    string(REGEX REPLACE "[\n\t ]+" " " CONTENT "R\"(${CONTENT})\"")
    set(${VAR_NAME} ${CONTENT} PARENT_SCOPE)
endfunction()

define_query(QUERY_GET_RANK   Database/query_get_rank.sql  )
define_query(QUERY_INIT       Database/query_init.sql      )
define_query(QUERY_UPDATE_LB  Database/query_update_lb.sql )
define_query(QUERY_GET_TOP_10 Database/query_get_top_10.sql)
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
configure_file(Database/Queries.h.in ${PROJECT_SOURCE_DIR}/Database/Queries.h)
