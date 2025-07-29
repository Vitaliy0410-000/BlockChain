cmake_minimum_required(VERSION 3.16)

project(testChain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(testChain main.cpp
    Block.h
    Block.cpp

    Logger.cpp
    Logger.h
    BlockFactory.h BlockFactory.cpp
    RegularBlock.h RegularBlock.cpp
    BlockChain.h BlockChain.cpp
    log.txt
    Transaction.h Transaction.cpp
    Blockk.cpp
    SmartContract.h SmartContract.cpp
    Virtual_Machine.h Virtual_Machine.cpp
)

find_package(nlohmann_json 3.12.0 REQUIRED)


if (nlohmann_json_FOUND)
    message(STATUS "nlohmann/json found. Linking to target testChain.")
else()
    message(FATAL_ERROR "nlohmann/json not found! Please install nlohmann/json development libraries.")
endif()


find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

# 2. Если OpenSSL найден, добавить его к нашему исполняемому файлу
if (OPENSSL_FOUND)
    message(STATUS "OpenSSL found. Including directories: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "Linking libraries: ${OPENSSL_LIBRARIES}")

    # Добавить инклюд-директории OpenSSL к нашему исполняемому файлу
    target_include_directories(testChain PRIVATE ${OPENSSL_INCLUDE_DIR})

    # Прилинковать библиотеки OpenSSL к нашему исполняемому файлу
    target_link_libraries(testChain PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json # Здесь добавляем nlohmann/json
    )
else()
    message(FATAL_ERROR "OpenSSL not found! Please install OpenSSL development libraries.")
endif()

include(GNUInstallDirs)
install(TARGETS testChain
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
