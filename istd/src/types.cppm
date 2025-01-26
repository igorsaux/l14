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

#include <cstdint>
#include <variant>

export module istd.types;

export using i8 = int8_t;
export using u8 = uint8_t;
export using i16 = int16_t;
export using u16 = uint16_t;
export using i32 = int32_t;
export using u32 = uint32_t;
export using i64 = int64_t;
export using u64 = uint64_t;

#ifdef L14_IS_WINDOWS
export using usize = size_t;
export using isize = ptrdiff_t;
#else
export using usize = std::size_t;
export using isize = std::ptrdiff_t;
#endif

export using f32 = float;
export using f64 = double;

export class Bool final
{
  public:
	constexpr Bool() = default;

	constexpr explicit Bool( const bool value ) : m_value( value )
	{
	}

	constexpr operator bool() const noexcept
	{
		return m_value;
	}

	constexpr bool operator==( const Bool other ) const noexcept
	{
		return m_value == other.m_value;
	}

	constexpr bool operator==( const bool other ) const noexcept
	{
		return m_value == other;
	}

  private:
	bool m_value;
};

export constexpr auto True = Bool( true );

export constexpr auto False = Bool( false );

export template <typename O, typename E>
using Result = std::variant<O, E>;
