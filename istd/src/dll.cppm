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

#if L14_IS_WINDOWS
	#include <windows.h>
#elif L14_IS_LINUX
	#include <dlfcn.h>
#else
	#error The OS is not supported
#endif

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

export module istd.dll;

import istd.crash;
import istd.platform;

namespace istd
{

export class IDll
{
  public:
	static std::pair<std::string, std::unique_ptr<IDll>>
	Open( const std::filesystem::path& path );

	[[nodiscard]] virtual void*
	FindProc( const std::string& name ) const noexcept = 0;

	template <typename T>
	[[nodiscard]] constexpr T FindProc( const std::string& name ) const noexcept
	{
		return reinterpret_cast<T>( FindProc( name ) );
	}

	virtual ~IDll() = default;
};

#if L14_IS_WINDOWS

class WindowsDll final : public IDll
{
  public:
	WindowsDll() = delete;

	WindowsDll( const WindowsDll& other ) = delete;

	WindowsDll( WindowsDll&& other ) noexcept
		: mLibPtr( std::exchange( other.mLibPtr, nullptr ) )
	{
	}

	WindowsDll& operator=( WindowsDll&& other ) noexcept
	{
		std::swap( mLibPtr, other.mLibPtr );

		return *this;
	}

	WindowsDll& operator=( const WindowsDll& other ) = delete;

	explicit WindowsDll( const HMODULE libPtr ) : mLibPtr( libPtr )
	{
	}

	[[nodiscard]] void*
	FindProc( const std::string& name ) const noexcept override
	{
		ASSERT( mLibPtr != nullptr );

		return GetProcAddress( mLibPtr, name.c_str() );
	}

	~WindowsDll() override
	{
		if ( mLibPtr == nullptr )
			return;

		FreeLibrary( std::exchange( mLibPtr, nullptr ) );
	}

  private:
	HMODULE mLibPtr = nullptr;
};

std::pair<std::string, std::unique_ptr<IDll>>
IDll::Open( const std::filesystem::path& path )
{
	const HMODULE libPtr = LoadLibraryW( path.c_str() );

	if ( libPtr == nullptr )
		return std::make_pair( win32::GetWindowsError(), nullptr );

	return std::make_pair( std::string{},
						   std::make_unique<WindowsDll>( libPtr ) );
}

#elif L14_IS_LINUX

class LinuxDll final : public IDll
{
  public:
	LinuxDll() = delete;

	LinuxDll( const LinuxDll& other ) = delete;

	LinuxDll( LinuxDll&& other ) noexcept
		: m_handle( std::exchange( other.m_handle, nullptr ) )
	{
	}

	LinuxDll& operator=( LinuxDll&& other ) noexcept
	{
		std::swap( m_handle, other.m_handle );

		return *this;
	}

	LinuxDll& operator=( const LinuxDll& other ) = delete;

	explicit LinuxDll( void* handle ) : m_handle( handle )
	{
	}

	[[nodiscard]] void*
	FindProc( const std::string& name ) const noexcept override
	{
		ASSERT( m_handle != nullptr );

		return dlsym( m_handle, name.c_str() );
	}

	~LinuxDll() override
	{
		if ( m_handle == nullptr )
		{
			return;
		}

		dlclose( std::exchange( m_handle, nullptr ) );
	}

  private:
	void* m_handle = nullptr;
};

std::pair<std::string, std::unique_ptr<IDll>>
IDll::Open( const std::filesystem::path& path )
{
	void* handle = dlopen( path.c_str(), RTLD_NOW );

	if ( handle == nullptr )
		return std::make_pair( std::string{ dlerror() }, nullptr );

	return std::make_pair( std::string{},
						   std::make_unique<LinuxDll>( handle ) );
}

#else
	#error The OS is not supported
#endif

} // namespace istd
