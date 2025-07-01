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
)

find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

# 2. Если OpenSSL найден, добавить его к нашему исполняемому файлу
if (OPENSSL_FOUND)
    message(STATUS "OpenSSL found. Including directories: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "Linking libraries: ${OPENSSL_LIBRARIES}")

    # Добавить инклюд-директории OpenSSL к нашему исполняемому файлу
    target_include_directories(testChain PRIVATE ${OPENSSL_INCLUDE_DIR})

    # Прилинковать библиотеки OpenSSL к нашему исполняемому файлу
    target_link_libraries(testChain PRIVATE ${OPENSSL_LIBRARIES})
else()
    message(FATAL_ERROR "OpenSSL not found! Please install OpenSSL development libraries.")
endif()

include(GNUInstallDirs)
install(TARGETS testChain
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
