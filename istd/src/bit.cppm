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

#include <algorithm>
#include <bit>
#include <concepts>
#include <type_traits>

export module istd.bit;

namespace istd::bit
{

//-----------------------------------------------------------------------------
// https://en.cppreference.com/w/cpp/numeric/byteswap
//-----------------------------------------------------------------------------
export template <std::integral T>
constexpr T ByteSwap( T value ) noexcept
{
	static_assert( std::has_unique_object_representations_v<T>,
				   "T may not have padding bits" );

	auto value_representation =
		std::bit_cast<std::array<std::byte, sizeof( T )>>( value );

	std::ranges::reverse( value_representation );

	return std::bit_cast<T>( value_representation );
}

} // namespace istd::bit
