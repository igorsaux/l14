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
#include <format>
#include <random>
#include <string>

export module istd.uuid;

import istd;

namespace istd
{

std::mt19937 g_rng{ std::random_device{}() };

export struct UuidV4
{
	static UuidV4 Generate() noexcept
	{
		std::array<u8, 16> data{};

		for ( unsigned char& i : data )
			i = static_cast<u8>( g_rng() );

		data[8] = ( data[8] & ~0xc0 ) | 0x80;
		data[6] = ( data[6] & ~0xf0 ) | 0x40;

		return UuidV4{ data };
	}

	[[nodiscard]] std::string ToString() const noexcept;

	std::array<u8, 16> data{};

	static const UuidV4 null;
};

const UuidV4 UuidV4::null = { std::array<u8, 16>{} };

} // namespace istd

export template <>
struct std::formatter<istd::UuidV4, char>
{
	template <class ParseContext>
	constexpr typename ParseContext::iterator parse( ParseContext& ctx )
	{
		return ctx.begin();
	}

	template <class FmtContext>
	typename FmtContext::iterator format( const istd::UuidV4& uuid,
										  FmtContext& ctx ) const
	{
		return std::format_to(
			ctx.out(),
			"{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:"
			"02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
			uuid.data[0], uuid.data[1], uuid.data[2], uuid.data[3],
			uuid.data[4], uuid.data[5], uuid.data[6], uuid.data[7],
			uuid.data[8], uuid.data[9], uuid.data[10], uuid.data[11],
			uuid.data[12], uuid.data[13], uuid.data[14], uuid.data[15] );
	}
};

std::string istd::UuidV4::ToString() const noexcept
{
	return std::format( "{}", *this );
}
