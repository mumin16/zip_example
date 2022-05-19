cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    set(SELECTDIR windows)
    set(SELECTSOURCE resource.rc)
    set(LIBRARIES mingw32)
elseif (UNIX)
    set(SELECTDIR linux)
    #set(LIBRARIES dl pthread)
elseif (APPLE)
    set(SELECTDIR macos)
    #set(LIBRARIES iconv)
endif ()
set(EXECUTABLE_OUTPUT_PATH "../bin")
set(CMAKE_C_FLAGS "-DHAVE_INTTYPES_H -DHAVE_STDINT_H -DHAVE_ZLIB -DMZ_ZIP_NO_CRYPTO -DMZ_ZIP_NO_ENCRYPTION -DZLIB_COMPAT -D_POSIX_C_SOURCE=200112L")
include_directories(../third_party/include ../third_party/include/${SELECTDIR})
link_directories(../third_party/lib/${SELECTDIR})

project(demo)
add_executable(demo  main.cpp

        ${SELECTSOURCE}

        ../third_party/include/zlib/adler32.c
        ../third_party/include/zlib/compress.c
        ../third_party/include/zlib/crc32.c
        ../third_party/include/zlib/deflate.c
        ../third_party/include/zlib/gzclose.c
        ../third_party/include/zlib/gzlib.c
        ../third_party/include/zlib/gzread.c
        ../third_party/include/zlib/gzwrite.c
        ../third_party/include/zlib/inflate.c
        ../third_party/include/zlib/infback.c
        ../third_party/include/zlib/inffast.c
        ../third_party/include/zlib/inftrees.c
        ../third_party/include/zlib/trees.c
        ../third_party/include/zlib/uncompr.c
        ../third_party/include/zlib/zutil.c

        ../third_party/include/minizip/mz_crypt.c
        ../third_party/include/minizip/mz_os.c
        ../third_party/include/minizip/mz_strm.c
        ../third_party/include/minizip/mz_strm_buf.c
        ../third_party/include/minizip/mz_strm_mem.c
        ../third_party/include/minizip/mz_strm_split.c
        ../third_party/include/minizip/mz_zip.c
        ../third_party/include/minizip/mz_zip_rw.c
        ../third_party/include/minizip/mz_strm_zlib.c
        ../third_party/include/minizip/mz_os_posix.c
        ../third_party/include/minizip/mz_strm_os_posix.c


        ) #demo sonrası WIN32 konsolu kapar


target_link_libraries(demo ${LIBRARIES})
