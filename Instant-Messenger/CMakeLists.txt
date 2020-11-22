cmake_minimum_required(VERSION 3.17)
project(Instant_Messenger)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include_directories(include/client)
include_directories(include/server)
include_directories(include/util)


find_package(Threads)

add_executable(Instant_Messenger
        include/client/client.h
        include/client/client_communication_manager.h
        include/client/client_message_manager.h
        include/client/user_interface.h
        include/server/server.hpp
        include/server/Group.hpp
        include/server/server_communication_manager.h
        include/server/server_group_manager.hpp
        include/server/server_message_manager.hpp
        include/util/definitions.hpp
        include/util/exceptions.hpp
        include/util/file_system_manager.hpp
        include/util/message.hpp
        include/util/Packet.hpp
        include/util/Semaphore.hpp
        include/util/Socket.hpp
        include/util/user.hpp
        src/app/app_server.cpp
        src/client/client.cpp
        src/client/client_communication.cpp
        src/client/client_message_manager.cpp
        src/client/user_interface.cpp
        src/server/server.cpp
        src/server/Group.cpp
        src/server/server_group_manager.cpp
        src/server/server_message_manager.cpp
        src/util/file_system_manager.cpp
        src/util/message.cpp
        src/util/Semaphore.cpp
        src/util/Socket.cpp
        src/util/user.cpp src/util/ConnectionKeeper.cpp include/util/ConnectionKeeper.hpp src/util/ConnectionMonitor.cpp include/util/ConnectionMonitor.hpp
        src/util/Uuid.cpp
        include/util/StringConstants.hpp src/util/StringConstants.cpp src/server/FeAddressBook.cpp include/server/FeAddressBook.hpp)

target_link_libraries( Instant_Messenger ${CMAKE_THREAD_LIBS_INIT})