// L14 - Space Station 14 Launcher
// Copyright (C) 2025 Igor Spichkin
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

module;

#include "istd/platform.hpp"

#ifdef L14_IS_WINDOWS
	#include <windows.h>
#elif L14_IS_LINUX
	#include <iostream>
#else
	#error The OS is not supported
#endif

#include <format>

export module istd.crash;

namespace istd
{

export L14_NORETURN inline void Crash( const char* file, int line,
									   const char* expression )
{
	auto msg =
		std::format( "{}:{} Assertion failed '{}'", file, line, expression );

#ifdef L14_IS_WINDOWS
	MessageBoxA( nullptr, msg.c_str(), nullptr, MB_OK | MB_ICONERROR );
#elif L14_IS_LINUX
	std::cerr << msg << std::endl;
#else
	#error The OS is not supported
#endif

	std::abort();
}

export L14_NORETURN inline void Crash( const char* msg )
{
#ifdef L14_IS_WINDOWS
	MessageBoxA( nullptr, msg, nullptr, MB_OK | MB_ICONERROR );
#elif L14_IS_LINUX
	std::cerr << msg << std::endl;
#else
	#error The OS is not supported
#endif

	std::abort();
}

} // namespace istd
