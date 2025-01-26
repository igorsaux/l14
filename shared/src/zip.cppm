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
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <zip.h>

export module l14.shared.zip;

import istd;
import istd.crash;

namespace l14::zip
{

class File final
{
  public:
	explicit File( zip_file_t* handle ) : m_handle( handle )
	{
	}

	File( const File& ) = delete;
	File& operator=( const File& ) = delete;

	File( File&& other ) noexcept
		: m_handle( std::exchange( m_handle, other.m_handle ) )
	{
	}

	File& operator=( File&& other ) noexcept
	{
		std::swap( m_handle, other.m_handle );

		return *this;
	}

	bool operator==( const std::nullptr_t& ) const
	{
		return m_handle == nullptr;
	}

	zip_file_t* operator*() const noexcept
	{
		return m_handle;
	}

	[[nodiscard]] std::string GetError() const noexcept
	{
		ASSERT( m_handle != nullptr );

		const auto error = zip_file_get_error( m_handle );

		if ( error == nullptr )
			return "";

		return zip_error_strerror( error );
	}

	~File()
	{
		if ( m_handle == nullptr )
			return;

		zip_fclose( std::exchange( m_handle, nullptr ) );
	}

  private:
	zip_file_t* m_handle;
};

export class Exception final : public std::exception
{
  public:
	Exception( std::string error ) : m_error( std::move( error ) )
	{
	}

	[[nodiscard]] const char* what() const noexcept override
	{
		return m_error.c_str();
	}

  private:
	std::string m_error;
};

export using ProgressCallback = std::function<void( usize extracted )>;

export class Archive final
{
  public:
	Archive() = default;

	Archive( const Archive& ) = delete;
	Archive& operator=( const Archive& ) = delete;

	Archive( Archive&& other ) noexcept
		: m_handle( std::exchange( m_handle, other.m_handle ) )
	{
	}

	Archive& operator=( Archive&& other ) noexcept
	{
		std::swap( m_handle, other.m_handle );

		return *this;
	}

	bool operator==( const std::nullptr_t& ) const
	{
		return m_handle == nullptr;
	}

	[[nodiscard]] static Archive Open( const std::string& path )
	{
		int err;
		const auto handle = zip_open( path.c_str(), ZIP_RDONLY, &err );

		if ( handle == nullptr )
		{
			zip_error_t _error;
			zip_error_init_with_code( &_error, err );

			const auto error = zip_error_strerror( &_error );

			zip_error_fini( &_error );

			throw Exception{ error };
		}

		return Archive{ handle };
	}

	void Extract( const std::filesystem::path& dst,
				  std::optional<ProgressCallback> callback ) const
	{
		ASSERT( m_handle != nullptr );

		const auto entriesCount = zip_get_num_entries( m_handle, 0 );

		ASSERT( entriesCount != -1 );

		usize extracted{};

		for ( i64 i = 0; i < entriesCount; i++ )
		{
			std::string entryName;

			{
				const auto _entryName = zip_get_name( m_handle, i, 0 );

				if ( _entryName == nullptr )
					ThrowError();

				entryName = std::string{ _entryName };
			}

			const auto file = File{ zip_fopen_index( m_handle, i, 0 ) };

			if ( file == nullptr )
				ThrowError();

			zip_stat_t stat;

			if ( zip_stat_index( m_handle, i, 0, &stat ) != 0 )
				ThrowError();

			std::string dstFilePath;

			{
				const auto _dstFilePath = dst / entryName;

				std::filesystem::create_directories(
					_dstFilePath.parent_path() );

				dstFilePath = _dstFilePath.string();
			}

			std::ofstream dstFileStream{ dstFilePath,
										 std::ios::binary | std::ios::trunc };
			dstFileStream.exceptions( std::ios::badbit );

			while ( true )
			{
				std::array<char, 64000> buffer{};

				const auto bytes =
					zip_fread( *file, buffer.data(), buffer.size() );

				if ( bytes == 0 )
					break;

				if ( bytes == -1 )
					ThrowError();

				for ( i64 k = 0; k < bytes; k++ )
					dstFileStream << buffer[k];

				extracted += bytes;

				if ( callback.has_value() )
					( *callback )( extracted );
			}
		}
	}

	~Archive()
	{
		if ( m_handle == nullptr )
			return;

		zip_close( std::exchange( m_handle, nullptr ) );
	}

  private:
	explicit Archive( zip_t* handle ) : m_handle( handle )
	{
	}

	void ThrowError() const
	{
		const auto error = zip_get_error( m_handle );

		ASSERT( error != nullptr );

		throw Exception{ zip_error_strerror( error ) };
	}

	zip_t* m_handle{};
};

} // namespace l14::zip
