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

#pragma once

#define UNREACHABLE()                                                          \
	[[unlikely]] {                                                             \
		istd::Crash( __FILE__, __LINE__, "UNREACHABLE" );                      \
	}

#define ASSERT( EXPR )                                                         \
	if ( !( EXPR ) ) [[unlikely]]                                              \
	{                                                                          \
		istd::Crash( __FILE__, __LINE__, #EXPR );                              \
	}

#define ASSERT_MSG( EXPR, MSG )                                                \
	if ( !( EXPR ) ) [[unlikely]]                                              \
	{                                                                          \
		istd::Crash( MSG );                                                    \
	}

#define ASSERT_FMT( EXPR, ... )                                                \
	if ( !( EXPR ) ) [[unlikely]]                                              \
	{                                                                          \
		auto msg = std::format( __VA_ARGS__ );                                 \
		istd::Crash( msg.c_str() );                                            \
	}
