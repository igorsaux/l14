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

#include <charconv>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

export module istd.str;

import istd.types;

namespace istd::str
{

export [[nodiscard]] constexpr bool IsSpace( const char chr ) noexcept
{
	return chr == ' ' || chr == '\n' || chr == '\r' || chr == '\t' ||
		   chr == '\v' || chr == '\f';
}

export [[nodiscard]] constexpr bool IsDigit( const char chr ) noexcept
{
	return chr >= 48 && chr <= 57;
}

export [[nodiscard]] constexpr bool IsAsciiUpper( const char chr ) noexcept
{
	return chr >= 65 && chr <= 90;
}

export [[nodiscard]] constexpr bool IsAsciiLower( const char chr ) noexcept
{
	return chr >= 97 && chr <= 122;
}

export [[nodiscard]] constexpr bool IsAsciiLetter( const char chr ) noexcept
{
	return IsAsciiUpper( chr ) || IsAsciiLower( chr );
}

export [[nodiscard]] constexpr std::string
Escape( const std::string_view input, const std::string_view symbols ) noexcept
{
	if ( input.empty() || symbols.empty() )
		return std::string{};

	std::string result;
	result.reserve( input.size() );

	for ( const char chr : input )
	{
		if ( chr == '\0' )
			break;

		if ( symbols.find( chr ) != std::string::npos )
		{
			result += '\\';
			result += chr;
		}
		else
		{
			result += chr;
		}
	}

	return result;
}

export [[nodiscard]] constexpr std::string
UnEscape( const std::string_view input,
		  const std::string_view symbols ) noexcept
{
	if ( input.empty() || symbols.empty() )
		return std::string{};

	std::string result;
	result.reserve( input.size() );

	size_t i = 0;

	while ( i < input.size() )
	{
		const auto chr = input[i];

		if ( chr == '\0' )
			break;

		if ( chr == '\\' && i + 1 < input.size() )
		{
			if ( symbols.find( input[i + 1] ) != std::string::npos )
			{
				result += input[i + 1];
				i += 2;

				continue;
			}
		}

		result += chr;
		i++;
	}

	return result;
}

export [[nodiscard]] std::vector<std::string_view>
Split( const std::string_view input, const std::string_view delims,
	   const usize n = 0 ) noexcept
{
	if ( input.empty() || delims.empty() )
		return std::vector{ input };

	std::vector<std::string_view> result{};

	if ( n != 0 )
		result.reserve( n );

	isize last = 0;
	isize pos = 0;
	size_t times = 0;

	while ( pos < static_cast<isize>( input.size() ) )
	{
		if ( n != 0 && times == n )
		{
			pos = static_cast<isize>( input.size() );

			break;
		}

		const auto chr = input[pos];

		if ( chr == '\0' )
			break;

		if ( std::ranges::find( delims, chr ) != delims.end() )
		{
			if ( pos - last > 0 )
			{
				result.push_back( input.substr( last, pos - last ) );

				if ( n != 0 )
					times += 1;
			}

			last = pos + 1;
		}

		pos += 1;
	}

	if ( pos - last > 0 )
	{
		result.push_back( input.substr( last, pos - last ) );
	}

	return result;
}

export [[nodiscard]] std::vector<std::string_view>
SplitArgs( const std::string_view input ) noexcept
{
	enum class State
	{
		SearchingArgument,
		ParsingArgument,
	} state = State::SearchingArgument;

	if ( input.empty() )
		return std::vector<std::string_view>{};

	std::vector<std::string_view> result;
	usize start = 0;
	bool isQuoted = false;

	for ( size_t i = 0; i < input.size(); ++i )
	{
		const auto chr = input[i];

		if ( chr == '\0' )
			break;

		switch ( state )
		{
		case State::SearchingArgument:
			if ( IsSpace( chr ) )
				continue;

			if ( chr == '\"' )
			{
				start = i + 1;
				isQuoted = true;
			}
			else
				start = i;

			state = State::ParsingArgument;

			break;
		case State::ParsingArgument:
			if ( IsSpace( chr ) && !isQuoted )
			{
				result.push_back( input.substr( start, i - start ) );
				isQuoted = false;
				state = State::SearchingArgument;

				break;
			}

			if ( chr == '\"' && static_cast<isize>( i ) - 1 >= 0 &&
				 input[i - 1] != '\\' )
			{
				result.push_back( input.substr( start, i - start ) );
				state = State::SearchingArgument;
			}

			break;
		}
	}

	if ( state == State::ParsingArgument && input.size() - start > 0 )
		result.push_back( input.substr( start ) );

	return result;
}

export template <typename T>
	requires std::integral<T>
[[nodiscard]] std::optional<T> FromChars( const std::string_view input,
										  const int base = 10 ) noexcept
{
	if ( input.empty() )
		return std::nullopt;

	T value;

	const auto [_, ec] = std::from_chars(
		input.data(), input.data() + input.size(), value, base );

	if ( ec == std::errc{} )
		return value;

	return std::nullopt;
}

export template <typename T>
	requires std::floating_point<T>
[[nodiscard]] std::optional<T>
FromChars( const std::string_view input ) noexcept
{
	if ( input.empty() )
		return std::nullopt;

	T value;

	const auto [_, ec] =
		std::from_chars( input.data(), input.data() + input.size(), value );

	if ( ec == std::errc{} )
		return value;

	return std::nullopt;
}

export [[nodiscard]] constexpr std::string_view
TrimSpacesStart( const std::string_view input ) noexcept
{
	for ( size_t i = 0; i < input.size(); i++ )
	{
		if ( !IsSpace( input[i] ) )
			return input.substr( i );
	}

	return input;
}

export [[nodiscard]] constexpr std::string_view
TrimSpacesEnd( const std::string_view input ) noexcept
{
	for ( size_t i = input.size() - 1; i > 0; i-- )
	{
		if ( !IsSpace( input[i] ) )
			return input.substr( 0, i + 1 );
	}

	return input;
}

export [[nodiscard]] constexpr std::string_view
TrimSpaces( const std::string_view input ) noexcept
{
	return TrimSpacesStart( TrimSpacesEnd( input ) );
}

export [[nodiscard]] constexpr char ToLowerChar( const char chr ) noexcept
{
	if ( IsAsciiUpper( chr ) )
		return static_cast<char>( chr + 32 );

	return chr;
}

export constexpr void ToLower( std::string& input ) noexcept
{
	for ( size_t i = 0; i < input.size(); i++ )
		input[i] = ToLowerChar( input[i] );
}

export [[nodiscard]] constexpr std::string
ToLower( const std::string_view input ) noexcept
{
	if ( input.empty() )
		return std::string{};

	std::string result{};
	result.resize( input.size(), '\0' );

	for ( size_t i = 0; i < input.size(); i++ )
		result[i] = ToLowerChar( input[i] );

	return result;
}

export [[nodiscard]] constexpr bool
CompareInsensitive( const std::string_view left,
					const std::string_view right ) noexcept
{
	if ( left.size() != right.size() )
		return false;

	for ( size_t i = 0; i < left.size(); i++ )
	{
		const auto l = ToLowerChar( left[i] );
		const auto r = ToLowerChar( right[i] );

		if ( l != r )
			return false;
	}

	return true;
}

} // namespace istd::str
