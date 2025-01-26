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

#include <bit>
#include <concepts>
#include <span>
#include <vector>

export module istd.io;

import istd.bit;
import istd;

namespace istd
{

export class BufferReader final
{
  public:
	BufferReader( const std::span<const char> buffer ) : m_buffer( buffer )
	{
	}

	[[nodiscard]] std::span<const char> Read( const usize count ) noexcept
	{
		const auto dst = std::min( m_pos + count, m_buffer.size() );

		if ( End() || m_pos == dst )
			return std::span<char>{};

		const auto old_pos = m_pos;
		m_pos = dst;

		return std::span{ m_buffer.data() + old_pos, dst - old_pos };
	}

	template <std::integral T>
	[[nodiscard]] bool ReadNumber( T& out )
	{
		auto bytes = Read( sizeof( T ) );

		if ( bytes.size() != sizeof( T ) )
		{
			out = 0;

			return false;
		}

		std::array<char, sizeof( T )> _bytes{};

		for ( size_t i = 0; i < _bytes.size(); i++ )
			_bytes[i] = bytes[i];

		out = std::bit_cast<T>( _bytes );

		return true;
	}

	template <std::integral T>
	[[nodiscard]] bool ReadNumberLE( T& out )
	{
		if ( !ReadNumber( out ) )
			return false;

		if constexpr ( std::endian::native != std::endian::little )
			out = istd::bit::ByteSwap( out );

		return true;
	}

	template <std::integral T>
	[[nodiscard]] bool ReadNumberBE( T& out )
	{
		if ( !ReadNumber( out ) )
			return false;

		if constexpr ( std::endian::native != std::endian::big )
			out = istd::bit::ByteSwap( out );

		return true;
	}

	[[nodiscard]] bool End() const noexcept
	{
		return m_buffer.empty() || m_pos == m_buffer.size();
	}

  private:
	usize m_pos{};
	std::span<const char> m_buffer;
};

} // namespace istd
