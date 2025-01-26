find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBB2 REQUIRED IMPORTED_TARGET libb2)

add_library(shared STATIC)

target_sources(shared PUBLIC FILE_SET CXX_MODULES FILES
        src/app.cppm
        src/args.cppm
        src/b2.cppm
        src/client.cppm
        src/config.cppm
        src/constants.cppm
        src/http.cppm
        src/pbar.cppm
        src/sha256.cppm
        src/zip.cppm
)
target_link_libraries(shared PRIVATE
        istd
        CURL::libcurl
        nlohmann_json::nlohmann_json
        libzip::zip
        zstd::libzstd
        PkgConfig::LIBB2
        OpenSSL::SSL
        OpenSSL::Crypto
)
l14_add_target_opts(TARGET shared)
