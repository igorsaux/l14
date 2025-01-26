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
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

export module l14.client.convar;

import istd;
import istd.crash;

namespace l14
{

export class ConVarValue final
{
  public:
	ConVarValue( const double value ) : m_value( value )
	{
	}

	ConVarValue( const Bool value ) : m_value( value )
	{
	}

	ConVarValue( std::string value ) : m_value( std::move( value ) )
	{
	}

	ConVarValue( const char* value ) : m_value( std::string{ value } )
	{
	}

	bool operator==( const double other ) const noexcept
	{
		if ( IsNumber() )
			return GetNumber() == other;

		return false;
	}

	bool operator==( const bool other ) const noexcept
	{
		if ( IsBool() )
			return GetBool() == other;

		return false;
	}

	bool operator==( const Bool other ) const noexcept
	{
		if ( IsBool() )
			return GetBool() == other;

		return false;
	}

	bool operator==( const std::string_view other ) const noexcept
	{
		if ( IsString() )
			return GetString() == other;

		return false;
	}

	bool operator==( const ConVarValue& other ) const noexcept
	{
		return m_value == other.m_value;
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
	ConVarValue() = default;

	std::variant<double, Bool, std::string> m_value;
};

export class ConVarDef;

export class ConVarDef final
{
  public:
	ConVarDef() = delete;

	ConVarDef( std::string name, ConVarValue default_value,
			   std::string desc = "" ) noexcept
		: m_name( std::move( name ) ), m_defaultValue( default_value ),
		  m_value( std::move( default_value ) ), m_desc( std::move( desc ) )
	{
		m_next = s_instance;
		s_instance = this;
	}

	ConVarDef( const ConVarDef& ) = delete;
	ConVarDef( ConVarDef&& ) = delete;
	ConVarDef& operator=( const ConVarDef& ) = delete;
	ConVarDef& operator=( ConVarDef&& ) = delete;

	ConVarDef& operator=( ConVarValue value ) noexcept
	{
		m_value = std::move( value );

		return *this;
	}

	ConVarDef& operator=( const double value ) noexcept
	{
		m_value = value;

		return *this;
	}

	ConVarDef& operator=( const Bool value ) noexcept
	{
		m_value = value;

		return *this;
	}

	ConVarDef& operator=( std::string value ) noexcept
	{
		m_value = std::move( value );

		return *this;
	}

	bool operator==( const double value ) const noexcept
	{
		return m_value == value;
	}

	bool operator==( const Bool value ) const noexcept
	{
		return m_value == value;
	}

	bool operator==( const std::string_view value ) const noexcept
	{
		return m_value == value;
	}

	[[nodiscard]] ConVarDef* Next() const noexcept
	{
		return m_next;
	}

	[[nodiscard]] std::string_view Name() const noexcept
	{
		return m_name;
	}

	[[nodiscard]] const ConVarValue& DefaultValue() const noexcept
	{
		return m_defaultValue;
	}

	[[nodiscard]] const ConVarValue& Value() const noexcept
	{
		return m_value;
	}

	[[nodiscard]] std::string_view Desc() const noexcept
	{
		return m_desc;
	}

	[[nodiscard]] static ConVarDef* List() noexcept
	{
		return s_instance;
	}

  private:
	ConVarDef* m_next{};
	std::string m_name;
	ConVarValue m_defaultValue;
	ConVarValue m_value;
	std::string m_desc;

	static ConVarDef* s_instance;
};

ConVarDef* ConVarDef::s_instance = nullptr;

} // namespace l14

export template <>
struct std::formatter<l14::ConVarValue, char>
{
	template <class ParseContext>
	constexpr typename ParseContext::iterator parse( ParseContext& ctx )
	{
		return ctx.begin();
	}

	template <class FmtContext>
	typename FmtContext::iterator format( const l14::ConVarValue& value,
										  FmtContext& ctx ) const
	{
		if ( value.IsNumber() )
			return std::format_to( ctx.out(), "{}", value.GetNumber() );
		if ( value.IsBool() )
			return std::format_to( ctx.out(), "{}",
								   value.GetBool() ? "true" : "false" );
		if ( value.IsString() )
			return std::format_to( ctx.out(), "{}", value.GetString() );

		UNREACHABLE();
	}
};
