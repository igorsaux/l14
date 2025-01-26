add_library(istd STATIC)

target_sources(istd PUBLIC FILE_SET CXX_MODULES FILES
        src/a4.cppm
        src/bit.cppm
        src/crash.cppm
        src/defer.cppm
        src/dll.cppm
        src/format/kv.cppm
        src/fs.cppm
        src/hasher.cppm
        src/io.cppm
        src/istd.cppm
        src/literals.cppm
        src/platform.cppm
        src/str.cppm
        src/types.cppm
        src/uri.cppm
        src/uuid.cppm
)

target_include_directories(istd PUBLIC include/)
l14_add_target_opts(TARGET istd)

if (L14_ISTD_TESTS)
    find_package(Catch2 CONFIG REQUIRED)

    add_executable(tests_istd
            tests/io.cpp
            tests/kv.cpp
            tests/str.cpp
            tests/uri.cpp
    )
    l14_add_target_opts(TARGET tests_istd)
    target_link_libraries(tests_istd PRIVATE istd Catch2::Catch2WithMain)
endif ()
