cmake_minimum_required(VERSION 3.16)
project(testChain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Общий список исходных файлов (без main.cpp и main2.cpp)
set(SOURCE_FILES
    Block.h
    Blockk.cpp
    Block.cpp
    Logger.h
    Logger.cpp
    BlockFactory.h
    BlockFactory.cpp
    RegularBlock.h
    RegularBlock.cpp
    BlockChain.h
    BlockChain.cpp
    Transaction.h
    Transaction.cpp
    SmartContract.h
    SmartContract.cpp
    Virtual_Machine.h
    Virtual_Machine.cpp
    SmartContractParser.h
    SmartContractParser.cpp
    AstNode.h
    AstNode.cpp
    Node.h
    Node.cpp
)

# Сборка testChain (первая нода)
add_executable(testChain
    main.cpp
    ${SOURCE_FILES}
)
target_include_directories(testChain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(testChain PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    pthread
)

# Сборка testChain2 (вторая нода)
add_executable(testChain2
    main2.cpp
    ${SOURCE_FILES}
)
target_include_directories(testChain2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(testChain2 PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    pthread
)

# Поиск nlohmann_json
find_package(nlohmann_json REQUIRED)
if (nlohmann_json_FOUND)
    message(STATUS "nlohmann/json found. Linking to targets testChain and testChain2.")
else()
    message(FATAL_ERROR "nlohmann/json not found! Please install nlohmann/json development libraries.")
endif()

# Поиск OpenSSL
find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
if (OPENSSL_FOUND)
    message(STATUS "OpenSSL found. Including directories: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "Linking libraries: ${OPENSSL_LIBRARIES}")
    target_include_directories(testChain PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_include_directories(testChain2 PRIVATE ${OPENSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found! Please install OpenSSL development libraries.")
endif()

# Поиск Boost
find_package(Boost REQUIRED COMPONENTS system thread)
if (Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
    target_include_directories(testChain PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(testChain2 PRIVATE ${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found! Please install Boost libraries.")
endif()

# Установка
include(GNUInstallDirs)
install(TARGETS testChain testChain2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
