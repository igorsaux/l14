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

import istd.str;

TEST_CASE( "UnEscape", "[str]" )
{
	const auto text = "Hello, \\\"world!\\\"";
	const auto result = istd::str::UnEscape( text, "\"" );

	REQUIRE( result == "Hello, \"world!\"" );
}

TEST_CASE( "Split", "[str]" )
{
	{
		const auto input = "f;hello;world\nfoo\rbar\n\rbar2\r";
		const auto result = istd::str::Split( input, ";\n\r" );

		REQUIRE( result.size() == 6 );
		REQUIRE( result[0] == "f" );
		REQUIRE( result[1] == "hello" );
		REQUIRE( result[2] == "world" );
		REQUIRE( result[3] == "foo" );
		REQUIRE( result[4] == "bar" );
		REQUIRE( result[5] == "bar2" );
	}

	{
		const auto input = "Foo1:Foo2:Foo3";
		const auto result = istd::str::Split( input, ":", 1 );

		REQUIRE( result.size() == 2 );
		REQUIRE( result[0] == "Foo1" );
		REQUIRE( result[1] == "Foo2:Foo3" );
	}
}

TEST_CASE( "ArgsSplit", "[str]" )
{
	const auto args = istd::str::SplitArgs(
		"some_arg foo bar \"quoted text\" \"and another "
		"\\\"quoted\\\" text\"" );

	REQUIRE( args.size() == 5 );
	REQUIRE( args[0] == "some_arg" );
	REQUIRE( args[1] == "foo" );
	REQUIRE( args[2] == "bar" );
	REQUIRE( args[3] == "quoted text" );
	REQUIRE( args[4] == "and another \\\"quoted\\\" text" );

	const auto singleArg = istd::str::SplitArgs( "some_arg" );

	REQUIRE( singleArg.size() == 1 );
	REQUIRE( singleArg[0] == "some_arg" );
}

TEST_CASE( "TrimSpaces", "[str]" )
{
	const auto result = istd::str::TrimSpaces( " \tFoo Bar   \t " );

	REQUIRE( result == "Foo Bar" );
}

TEST_CASE( "ToLower", "[str]" )
{
	const auto result = istd::str::ToLower( "FoO BAR 123" );

	REQUIRE( result == "foo bar 123" );
}

TEST_CASE( "CompareInsensitive", "[str]" )
{
	REQUIRE( istd::str::CompareInsensitive( "1FOO BaR 2", "1foo bAr 2" ) );
}
