find_package(nlohmann_json CONFIG REQUIRED)

add_executable(cmd
        src/main.cpp
)
target_sources(cmd PRIVATE FILE_SET CXX_MODULES FILES
    src/process.cppm
    src/updater.cppm
)
set_target_properties(
        cmd
        PROPERTIES OUTPUT_NAME
        "l14cmd"
)
target_link_libraries(cmd PRIVATE istd shared nlohmann_json::nlohmann_json)
install(TARGETS cmd
        RUNTIME DESTINATION "."
)
l14_add_target_opts(TARGET cmd)
