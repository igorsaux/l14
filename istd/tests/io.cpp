// Space Station 14 Launcher
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

#include <catch2/catch_all.hpp>
#include <cstring>
#include <span>
#include <vector>

import istd.io;

using namespace istd;

TEST_CASE( "BufferReader", "[io]" )
{
	const std::vector data{ 'f', 'o', 'o', 'b', 'a', 'r' };
	BufferReader reader{ data };

	const auto foo = reader.Read( 3 );
	const auto bar = reader.Read( 3 );

	REQUIRE( reader.End() );

	REQUIRE( foo.size() == 3 );
	REQUIRE( std::memcmp( foo.data(), data.data(), 3 ) == 0 );

	REQUIRE( bar.size() == 3 );
	REQUIRE( std::memcmp( bar.data(), data.data() + 3, 3 ) == 0 );
}
