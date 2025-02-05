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

#include "istd/assert.hpp"

#ifdef L14_IS_WINDOWS
	#include <windows.h>
#elif L14_IS_LINUX
	#include <unistd.h>
#else
	#error The OS is not supported
#endif

#include <cstring>
#include <format>
#include <string>
#include <vector>

export module l14.cmd.process;

import istd.crash;
import istd.platform;
import istd;

namespace l14
{

#ifdef L14_IS_WINDOWS
constexpr std::string_view k_noupdateArg = " /noupdate";
#elif L14_IS_LINUX
constexpr std::string_view k_noupdateArg = "/noupdate";
#else
	#error The OS is not supported
#endif

export void RestartProcess( const int argc, char* argv[] ) noexcept
{
#ifdef L14_IS_WINDOWS
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory( &si, sizeof( STARTUPINFO ) );
	si.cb = sizeof( si );

	ZeroMemory( &pi, sizeof( PROCESS_INFORMATION ) );

	std::vector<char> args{};

	{

		const auto _args = GetCommandLineA();
		const auto size = std::strlen( _args );

		args.resize( size + k_noupdateArg.size() + 1, '\0' );

		std::memcpy( args.data(), _args, size );
		std::memcpy( args.data() + size, k_noupdateArg.data(),
					 k_noupdateArg.size() );
	}

	const auto ret = CreateProcessA( nullptr, args.data(), nullptr, nullptr,
									 FALSE, 0, nullptr, nullptr, &si, &pi );

	ASSERT_FMT( ret, "{}", istd::win32::GetWindowsError() );

	WaitForSingleObject( pi.hProcess, INFINITE );

	if ( pi.hProcess != nullptr )
		CloseHandle( pi.hProcess );

	if ( pi.hThread != nullptr )
		CloseHandle( pi.hThread );
#elif L14_IS_LINUX
	std::vector<std::string> args{};
	args.reserve( argc + 1 );
	std::vector<char*> _args{};
	_args.reserve( argc + 1 );

	{
		for ( int i = 0; i < argc; i++ )
		{
			args.emplace_back( argv[i] );
			_args.emplace_back( args.back().data() );
		}

		args.emplace_back( k_noupdateArg );
		_args.emplace_back( args.back().data() );

		_args.emplace_back( nullptr );
	}

	execv( argv[0], _args.data() );
#else
	#error The OS is not supported
#endif
}

} // namespace l14
