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

import istd.uri;

TEST_CASE( "URI Parsing", "[uri]" )
{
	const auto original =
		"https://example.com:8080/page/index.php?a=foo&b=bar#Desc";
	const auto uri = istd::Uri::Parse( original );

	REQUIRE( uri.has_value() );

	const auto& _uri = *uri;

	REQUIRE( _uri.Scheme() == "https" );
	REQUIRE( _uri.Authority() == "example.com" );
	REQUIRE( _uri.Port() == "8080" );
	REQUIRE( _uri.Path() == std::vector<std::string>{ "page", "index.php" } );
	REQUIRE( _uri.Query() == std::vector<std::pair<std::string, std::string>>{
								 { "a", "foo" }, { "b", "bar" } } );
	REQUIRE( _uri.Fragment() == "Desc" );
	REQUIRE( std::format( "{}", _uri ) == original );
}
