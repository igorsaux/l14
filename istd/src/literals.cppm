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

#include <string_view>

export module istd.literals;

import istd.types;

export constexpr u8 operator"" _u8( const unsigned long long value )
{
    return static_cast<u8>( value );
}

export constexpr u32 operator"" _u32( const unsigned long long value )
{
    return static_cast<u32>( value );
}

export constexpr u64 operator"" _u64( const unsigned long long value )
{
    return value;
}

export constexpr usize operator"" _usize( const unsigned long long value )
{
    return value;
}

export constexpr i8 operator"" _i8( const unsigned long long value )
{
    return static_cast<i8>( value );
}

export constexpr i32 operator"" _i32( const unsigned long long value )
{
    return static_cast<i32>( value );
}

export constexpr i64 operator"" _i64( const unsigned long long value )
{
    return static_cast<i64>( value );
}

export constexpr isize operator"" _isize( const unsigned long long value )
{
    return static_cast<isize>( value );
}
