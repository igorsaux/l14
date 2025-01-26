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

#include "istd/assert.hpp"
#include <cstring>
#include <locale>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

export module l14.shared.args;

import istd;
import istd.str;
import istd.crash;

namespace l14
{

export class ArgValue final
{
  public:
	ArgValue() = delete;

	ArgValue( const char* value ) : m_value( std::string{ value } )
	{
	}

	ArgValue( std::string value ) : m_value( std::move( value ) )
	{
	}

	ArgValue( const double value ) : m_value( value )
	{
	}

	ArgValue( const Bool value ) : m_value( value )
	{
	}

	bool operator==( const ArgValue& other ) const noexcept
	{
		return m_value == other.m_value;
	}

	bool operator==( const double other ) const noexcept
	{
		if ( !IsNumber() )
			return false;

		return GetNumber() == other;
	}

	bool operator==( const Bool other ) const noexcept
	{
		if ( !IsBool() )
			return false;

		return GetBool() == other;
	}

	bool operator==( const char* other ) const noexcept
	{
		if ( !IsString() )
			return false;

		return std::strcmp( other, GetString().c_str() ) == 0;
	}

	[[nodiscard]] bool IsNumber() const noexcept
	{
		return std::holds_alternative<double>( m_value );
	}

	[[nodiscard]] bool IsBool() const noexcept
	{
		return std::holds_alternative<Bool>( m_value );
	}

	[[nodiscard]] bool IsString() const noexcept
	{
		return std::holds_alternative<std::string>( m_value );
	}

	[[nodiscard]] double GetNumber() const noexcept
	{
		ASSERT( IsNumber() );

		return std::get<double>( m_value );
	}

	[[nodiscard]] Bool GetBool() const noexcept
	{
		ASSERT( IsBool() );

		return std::get<Bool>( m_value );
	}

	[[nodiscard]] const std::string& GetString() const noexcept
	{
		ASSERT( IsString() );

		return std::get<std::string>( m_value );
	}

	[[nodiscard]] std::string& GetString() noexcept
	{
		ASSERT( IsString() );

		return std::get<std::string>( m_value );
	}

  private:
	std::variant<double, Bool, std::string> m_value;
};

export class Args final
{
  public:
	void Parse( const int argc, char* argv[] ) noexcept
	{
		std::vector<std::string_view> args{};
		args.reserve( argc );

		for ( auto i = 0; i < argc; i++ )
			args.emplace_back( argv[i] );

		Parse( args );
	}

	void Parse( const std::span<const std::string_view> args ) noexcept
	{
		for ( size_t i = 0; i < args.size(); i++ )
		{
			const auto argI = istd::str::TrimSpaces( args[i] );

			if ( !argI.starts_with( '-' ) )
				continue;

			const auto key = std::string{ argI.substr( 1 ) };

			if ( i == args.size() - 1 ||
				 istd::str::TrimSpacesStart( args[i + 1] ).starts_with( '-' ) )
			{
				m_args.insert_or_assign( key, True );

				continue;
			}

			const auto value = istd::str::TrimSpaces( args[i + 1] );

			if ( value == "true" )
				m_args.insert_or_assign( key, True );
			else if ( value == "false" )
				m_args.insert_or_assign( key, False );
			else if ( const auto number = istd::str::FromChars<double>( value );
					  number.has_value() )
			{
				m_args.insert_or_assign( key, *number );
			}
			else
			{
				m_args.insert_or_assign( key, std::string{ value } );
			}
		}
	}

	[[nodiscard]] const ArgValue*
	Find( const std::string_view name ) const noexcept
	{
		const auto value = m_args.find( std::string{ name } );

		if ( value == m_args.end() )
			return nullptr;

		return &value->second;
	}

  private:
	std::unordered_map<std::string, ArgValue> m_args{};
};

} // namespace l14
