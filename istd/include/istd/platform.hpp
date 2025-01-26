// Space Station 14 Launcher
// Copyright (C) 2025 Igor Spichkin
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implL14d warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#ifdef L14_IS_WINDOWS
    #define L14_DLL_EXPORT extern "C" __declspec( dllexport )
    #define L14_DLL_IMPORT extern "C" __declspec( dllimport )

    #define L14_DLL_CLASS_EXPORT __declspec( dllexport )
    #define L14_DLL_CLASS_IMPORT __declspec( dllimport )

    #define L14_DLL_GLOBAL_EXPORT extern __declspec( dllexport )
    #define L14_DLL_GLOBAL_IMPORT extern __declspec( dllimport )
#elif L14_IS_LINUX
    #define L14_DLL_EXPORT                                                    \
        extern "C" __attribute__( ( visibility( "default" ) ) )
    #define L14_DLL_IMPORT extern "C"

    #define L14_DLL_CLASS_EXPORT __attribute__( ( visibility( "default" ) ) )
    #define L14_DLL_CLASS_IMPORT

    #define L14_DLL_GLOBAL_EXPORT                                             \
        extern __attribute( ( visibility( "default" ) ) )
    #define L14_DLL_GLOBAL_IMPORT extern
#else
    #error OS is not supported
#endif

#ifdef L14_IS_MSVC
    #define L14_NORETURN __declspec( noreturn )
#elif L14_IS_CLANG
    #define L14_NORETURN __attribute__((noreturn))
#else
    #define L14_NORETURN
#endif
