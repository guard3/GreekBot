cmake_minimum_required(VERSION 3.17)
project(GreekBot)

set(CMAKE_CXX_STANDARD 17)

add_executable(GreekBot main.cpp boost/beast.h boost/json.h Discord/Bot.cpp Discord/Bot.h Discord/Discord.cpp Discord/Discord.h Discord/Event.cpp Discord/Event.h Discord/Types.h Discord/Gateway/Gateway.cpp Discord/Gateway/Gateway.h Discord/Gateway/GatewayInfo.h Discord/Interaction/Interaction.cpp Discord/Interaction/Interaction.h Discord/Objects/Member.cpp Discord/Objects/Member.h Discord/Objects/User.cpp Discord/Objects/User.h Net/Net.cpp Net/Net.h Net/Websocket.cpp Net/Websocket.h Utils/Utils.cpp Utils/Utils.h Discord/Objects/InteractionResponse.h Discord/Objects/Component.h Discord/Objects/Emoji.h Discord/Interaction/ApplicationCommandInteractionDataOption.cpp Discord/Interaction/ApplicationCommandInteractionData.cpp Discord/Interaction/MessageComponentInteractionData.cpp Discord/Voice/VoiceGateway.h Discord/Objects/Guild.h Discord/Voice/VoiceState.h Discord/Objects/Channel.h Discord/Objects/Message.h Database.h Database.cpp)

target_include_directories(GreekBot PRIVATE ${PROJECT_SOURCE_DIR}/boost ${PROJECT_SOURCE_DIR}/Discord ${PROJECT_SOURCE_DIR}/Discord/Objects ${PROJECT_SOURCE_DIR}/Discord/Gateway ${PROJECT_SOURCE_DIR}/Discord/Interaction ${PROJECT_SOURCE_DIR}/Discord/Voice ${PROJECT_SOURCE_DIR}/Net ${PROJECT_SOURCE_DIR}/Utils)

# Link OpenSSL
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
target_link_libraries(GreekBot OpenSSL::SSL)

# Link Boost
set(Boost_USE_STATIC_LIBS     ON)
set(Boost_USE_DEBUG_LIBS     OFF)
set(Boost_USE_RELEASE_LIBS    ON)
set(Boost_USE_MULTITHREADED   ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.76.0 REQUIRED COMPONENTS json)
target_link_libraries(GreekBot ${Boost_LIBRARIES})

# Link SQLite
find_package(SQLite3 REQUIRED)
target_link_libraries(GreekBot SQLite::SQLite3)