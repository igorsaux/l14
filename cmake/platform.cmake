set(L14_PROJECT_COMPILER_DEFINES)
set(L14_PROJECT_COMPILER_OPTIONS)

block(PROPAGATE PROJECT_COMPILER_DEFINES PROJECT_COMPILER_OPTIONS)
    set(SUPPORTED_PLATFORMS "x86_64" "AMD64")

    if (WIN32) # Windows specific
        list(APPEND PROJECT_COMPILER_DEFINES L14_IS_WINDOWS)
    elseif (LINUX) # Linux specific
        list(APPEND PROJECT_COMPILER_DEFINES L14_IS_LINUX)
    else () # Not supported
        cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)
        message(FATAL_ERROR "The OS is not supported: ${OS_NAME}")
    endif ()

    cmake_host_system_information(RESULT OS_PLATFORM QUERY OS_PLATFORM)

    set(GNU_LIKE_COMPILERS "GNU" "Clang")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # MSVC specific
        list(APPEND PROJECT_COMPILER_DEFINES L14_IS_MSVC)

        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            list(APPEND PROJECT_COMPILER_OPTIONS "/MDd")
            list(APPEND PROJECT_COMPILER_OPTIONS "/RTCsu")
        else ()
            list(APPEND PROJECT_COMPILER_OPTIONS "/MD")
        endif ()

        list(APPEND PROJECT_COMPILER_OPTIONS "/Wall" "/guard:cf" "/DYNAMICBASE" "/EHscr" "/Qpar" "/sdl" "/utf-8")

        # Disable some warnings
        list(APPEND PROJECT_COMPILER_OPTIONS "/wd5045" "/wd4868" "/wd4365" "/wd4625" "/wd4626" "/wd4668" "/wd5026")
        list(APPEND PROJECT_COMPILER_OPTIONS "/wd5027" "/wd4686" "/wd4820" "/wd4061" "/wd5039" "/wd4623" "/wd4514")
        list(APPEND PROJECT_COMPILER_OPTIONS "/wd4711" "/wd4710")

        if (L14_ENABLE_SANITIZER)
            list(APPEND PROJECT_COMPILER_OPTIONS "/fsanitize=address")
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID IN_LIST GNU_LIKE_COMPILERS) # GCC or Clang specific
        list(APPEND PROJECT_COMPILER_OPTIONS "-Wall" "-Werror=format" "-Wformat=2" "-Wconversion" "-Wimplicit-fallthrough")
        list(APPEND PROJECT_COMPILER_OPTIONS "-Werror=format-security" "-fstrict-flex-arrays=3")
        list(APPEND PROJECT_COMPILER_OPTIONS "-fstack-clash-protection" "-fstack-protector-strong")
        list(APPEND PROJECT_COMPILER_OPTIONS "-fcf-protection=full" "-fno-delete-null-pointer-checks")
        list(APPEND PROJECT_COMPILER_OPTIONS "-Wstrict-aliasing" "-ftrivial-auto-var-init=pattern")
        list(APPEND PROJECT_COMPILER_DEFINES "-D_FORTIFY_SOURCE=3" "-DGLIBCXX_ASSERTIONS")

        if (L14_ENABLE_SANITIZER)
            list(APPEND PROJECT_COMPILER_OPTIONS "-fsanitize=address")
        endif ()

        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # GCC specific
            list(APPEND PROJECT_COMPILER_DEFINES L14_IS_GCC)

            list(APPEND PROJECT_COMPILER_OPTIONS "-Wl,-z,relro" "-Wl,-z,now")
            list(APPEND PROJECT_COMPILER_OPTIONS "-Wbidi-chars=any" "-fno-strict-overflow")
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # Clang specific
            list(APPEND PROJECT_COMPILER_DEFINES L14_IS_CLANG)

            list(APPEND PROJECT_COMPILER_OPTIONS "-Wno-sign-conversion")
        endif ()
    else () # Not supported
        message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} is not supported")
    endif ()
endblock()

function(l14_add_target_opts)
    cmake_parse_arguments(PARSE_ARGV 0 L14_ADD_TARGET "" "TARGET" "DEFINES;OPTIONS")

    set(TARGET ${L14_ADD_TARGET_TARGET})
    set(DEFINES ${L14_ADD_TARGET_DEFINES})
    set(OPTIONS ${L14_ADD_TARGET_OPTIONS})

    list(APPEND DEFINES ${PROJECT_COMPILER_DEFINES})
    list(APPEND OPTIONS ${PROJECT_COMPILER_OPTIONS})

    target_compile_definitions(
            ${TARGET}
            PRIVATE ${DEFINES}
    )
    target_compile_options(
            ${TARGET}
            PRIVATE ${OPTIONS}
    )
endfunction()
