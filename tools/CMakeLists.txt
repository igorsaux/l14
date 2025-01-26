add_executable(b2
        b2.cpp
)
target_link_libraries(b2 PRIVATE istd shared)
l14_add_target_opts(TARGET b2)
