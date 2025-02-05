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

#include <span>
#include <string>
#include <vector>
#include <zstd.h>

export module l14.shared.zstd;

namespace l14
{

export class Zstd final
{
  public:
	class Exception final : public std::exception
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

	static size_t Decompress( const std::span<const char> src,
							  std::vector<char>& dst )
	{
		const auto ret =
			ZSTD_decompress( dst.data(), dst.size(), src.data(), src.size() );

		if ( ZSTD_isError( ret ) )
			throw Exception{ ZSTD_getErrorName( ret ) };

		return ret;
	}
};

} // namespace l14
