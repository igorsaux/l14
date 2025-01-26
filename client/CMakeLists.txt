find_package(nlohmann_json CONFIG REQUIRED)

add_library(client MODULE)
target_sources(client PRIVATE FILE_SET CXX_MODULES FILES
        src/client.cppm
        src/cmd.cppm
        src/cmd_commands.cppm
        src/cmd_convars.cppm
        src/cmd_downloader.cppm
        src/cmd_server.cppm
        src/command.cppm
        src/convar.cppm
        src/globals.cppm
)
set_target_properties(
        client
        PROPERTIES OUTPUT_NAME
        "l14"
)
target_link_libraries(client PRIVATE istd shared nlohmann_json::nlohmann_json)
install(TARGETS client
        LIBRARY DESTINATION "."
)
l14_add_target_opts(TARGET client)
