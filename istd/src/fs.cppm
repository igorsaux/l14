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

#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <vector>

export module istd.fs;

import istd;
import istd.hasher;

namespace istd::fs
{

export [[nodiscard]]
std::filesystem::path GetAbsolutePath(
	const std::string_view path,
	const std::optional<std::filesystem::path>& base = std::nullopt )
{
	auto file_path = std::filesystem::path{ path };

	const auto old = std::filesystem::current_path();

	if ( base.has_value() )
		std::filesystem::current_path( base.value() );

	file_path = std::filesystem::weakly_canonical( file_path );

	current_path( old );

	return file_path;
}

export [[nodiscard]] bool Exists( const std::filesystem::path& path )
{
	return std::filesystem::exists( path ) &&
		   !std::filesystem::is_symlink( path );
}

export [[nodiscard]] std::vector<char> ReadAll( std::ifstream& stream )
{
	stream.seekg( 0, std::ios::end );

	const auto fileLength = stream.tellg();

	stream.seekg( 0, std::ios::beg );

	if ( fileLength == -1 )
		return std::vector<char>{};

	size_t pos = 0;
	std::vector result( fileLength, '\0' );

	while ( !stream.eof() )
	{
		auto* dst = result.data() + pos;
		const auto size = result.size() - pos;

		if ( size == 0 )
			break;

		stream.read( dst, static_cast<std::streamsize>( size ) );

		const auto bytes = static_cast<usize>( stream.gcount() );
		pos += bytes;
	}

	return result;
}

export void WriteAll( std::ofstream& stream,
					  const std::span<const char> content )
{
	for ( const char i : content )
		stream << i;

	stream.flush();
}

export [[nodiscard]] std::vector<char>
ComputeHash( const std::filesystem::path& path, IHasher& hasher )
{
	std::ifstream stream{ path, std::ios::binary };
	stream.exceptions( std::ios::badbit );

	while ( !stream.eof() )
	{
		std::array<char, 64000> buffer{};

		stream.read( buffer.data(), buffer.size() );

		const usize bytes = stream.gcount();

		if ( bytes == 0 )
			break;

		hasher.Update( std::span{ buffer.data(), bytes } );
	}

	return hasher.Final();
}

} // namespace istd::fs
