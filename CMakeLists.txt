cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(SELECTDIR windows)
    #set(SELECTSOURCE resource.rc)
    set(LIBRARIES mingw32)
elseif (UNIX)
    set(SELECTDIR linux)
    #set(LIBRARIES dl pthread)
elseif (APPLE)
    set(SELECTDIR macos)
    #set(LIBRARIES iconv)
endif ()
set(EXECUTABLE_OUTPUT_PATH "../bin")
#set(CMAKE_C_FLAGS "-DHAVE_INTTYPES_H -DHAVE_STDINT_H -DHAVE_ZLIB -DMZ_ZIP_NO_CRYPTO -DMZ_ZIP_NO_ENCRYPTION -DZLIB_COMPAT -D_POSIX_C_SOURCE=200112L")
include_directories(../third_party/include ../third_party/include/${SELECTDIR})
link_directories(../third_party/lib/${SELECTDIR})

project(demo)
add_executable(demo  main.cpp

        ${SELECTSOURCE}

        ../third_party/include/ziputil.cpp
        ) #demo sonrası WIN32 konsolu kapar


target_link_libraries(demo ${LIBRARIES} minizip z)
