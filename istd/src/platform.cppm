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

#ifdef L14_IS_WINDOWS
	#include <windows.h>
#elif L14_IS_LINUX
#else
	#error The OS is not supported
#endif

#include <string>

export module istd.platform;

namespace istd
{

#ifdef L14_IS_WINDOWS
namespace win32
{
export [[nodiscard]] std::string GetWindowsError()
{
	const auto code = GetLastError();
	LPSTR buffer = nullptr;

	const auto size = FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		reinterpret_cast<LPSTR>( &buffer ), 0, nullptr );

	// Remove \r\n
	auto str = std::string{ buffer, size - 2 };

	LocalFree( buffer );

	return str;
}
} // namespace win32
#elif L14_IS_LINUX
namespace linux
{

}
#else
	#error The OS is not supported
#endif

} // namespace istd