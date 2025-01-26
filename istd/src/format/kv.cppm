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
#include <algorithm>
#include <charconv>
#include <cstring>
#include <format>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module istd.format.kv;

import istd.a4;
import istd.str;
import istd.crash;
import istd;

namespace istd::format::kv
{

export class Value;

export using Object = std::unordered_map<std::string, Value>;

export using Array = std::vector<Value>;

export class Value final
{
  public:
	constexpr Value() = default;

	constexpr Value( const std::string_view value )
		: m_value( std::string{ value } )
	{
	}

	constexpr Value( const char* value ) : m_value( value )
	{
	}

	constexpr Value( std::string&& value ) : m_value( std::move( value ) )
	{
	}

	constexpr Value( const double value ) : m_value( value )
	{
	}

	constexpr Value( Object&& value ) : m_value( std::move( value ) )
	{
	}

	constexpr Value( Array&& value ) : m_value( std::move( value ) )
	{
	}

	bool operator==( const Value& other ) const
	{
		if ( IsString() && other.IsString() )
			return GetString() == other.GetString();

		if ( IsNumber() && other.IsNumber() )
			return GetNumber() == other.GetNumber();

		if ( IsBool() && other.IsBool() )
			return GetBool() == other.GetBool();

		if ( IsObject() && other.IsObject() )
			return GetObject() == other.GetObject();

		if ( IsArray() && other.IsArray() )
			return GetArray() == other.GetArray();

		UNREACHABLE();
	}

	[[nodiscard]] static Value FromBool( const bool value ) noexcept
	{
		auto v = Value{};
		v.m_value = value;

		return v;
	}

	[[nodiscard]] static Value EmptyObject() noexcept
	{
		return Value{ Object{} };
	}

	[[nodiscard]] static Value EmptyArray() noexcept
	{
		return Value{ Array{} };
	}

	[[nodiscard]] bool IsString() const noexcept
	{
		return std::holds_alternative<std::string>( m_value );
	}

	[[nodiscard]] bool IsNumber() const noexcept
	{
		return std::holds_alternative<double>( m_value );
	}

	[[nodiscard]] bool IsBool() const noexcept
	{
		return std::holds_alternative<bool>( m_value );
	}

	[[nodiscard]] bool IsObject() const noexcept
	{
		return std::holds_alternative<Object>( m_value );
	}

	[[nodiscard]] bool IsArray() const noexcept
	{
		return std::holds_alternative<Array>( m_value );
	}

	[[nodiscard]] std::string& GetString() noexcept
	{
		ASSERT( IsString() );

		return std::get<std::string>( m_value );
	}

	[[nodiscard]] const std::string& GetString() const noexcept
	{
		ASSERT( IsString() );

		return std::get<std::string>( m_value );
	}

	[[nodiscard]] double GetNumber() const noexcept
	{
		ASSERT( IsNumber() );

		return std::get<double>( m_value );
	}

	[[nodiscard]] bool GetBool() const noexcept
	{
		ASSERT( IsBool() );

		return std::get<bool>( m_value );
	}

	[[nodiscard]] Object& GetObject() noexcept
	{
		ASSERT( IsObject() );

		return std::get<Object>( m_value );
	}

	[[nodiscard]] const Object& GetObject() const noexcept
	{
		ASSERT( IsObject() );

		return std::get<Object>( m_value );
	}

	[[nodiscard]] Array& GetArray() noexcept
	{
		ASSERT( IsArray() );

		return std::get<Array>( m_value );
	}

	[[nodiscard]] const Array& GetArray() const noexcept
	{
		ASSERT( IsArray() );

		return std::get<Array>( m_value );
	}

	[[nodiscard]] std::string&& IntoString() && noexcept
	{
		ASSERT( IsString() );

		return std::move( std::get<std::string>( m_value ) );
	}

	[[nodiscard]] Object&& IntoObject() && noexcept
	{
		ASSERT( IsObject() );

		return std::move( std::get<Object>( m_value ) );
	}

	[[nodiscard]] Array&& IntoArray() && noexcept
	{
		ASSERT( IsArray() );

		return std::move( std::get<Array>( m_value ) );
	}

  private:
	std::variant<std::string, double, bool, Object, Array> m_value{};
};

export void Merge( Object& dst, Object&& src ) noexcept
{
	for ( auto& [key, value] : src )
	{
		auto dst_entry = dst.find( key );

		if ( dst_entry != dst.end() && dst_entry->second.IsObject() &&
			 value.IsObject() )
		{
			// TODO: Inline or depth guard
			Merge( dst_entry->second.GetObject(),
				   std::move( value ).IntoObject() );
		}
		else
		{
			dst.insert_or_assign( std::move( key ), std::move( value ) );
		}
	}
}

export struct ParsingError final
{
	usize position;
	std::string message;
};

export std::pair<Value, std::optional<ParsingError>>
Parse( const std::string_view content ) noexcept
{
	enum class State
	{
		SearchingKey,
		ParsingKey,
		SearchingValue,
		ParsingNumber,
		ParsingString,
	};

	struct Context
	{
		Value value;
		usize key_start = 0;
		usize value_start = 0;
		std::string_view key{};
	};

	usize cursor = 0;
	auto state = State::SearchingKey;
	auto contexts = std::list{ Context{ .value = Value::EmptyObject() } };

	const auto tryParseText = [&cursor,
							   &content]( const std::string_view text ) -> bool
	{
		if ( cursor + text.size() >= content.size() )
			return false;

		if ( std::memcmp( text.data(), content.data() + cursor, text.size() ) !=
			 0 )
		{
			return false;
		}

		cursor += text.size();

		return true;
	};

	const auto skipLine = [&cursor, &content]
	{
		while ( cursor < content.size() )
		{
			if ( content[cursor] == '\n' )
				return;

			cursor += 1;
		}
	};

	const auto skipSpaces = [&cursor, &content]() -> bool
	{
		while ( cursor < content.size() && str::IsSpace( content[cursor] ) )
		{
			cursor += 1;
		}

		return cursor < content.size();
	};

	while ( cursor < content.size() )
	{
		auto& ctx = contexts.back();
		char chr = content[cursor];

		if ( chr == '\0' )
			break;

		switch ( state )
		{
		case State::SearchingKey:
		{
			if ( !skipSpaces() )
				break;

			if ( tryParseText( "//" ) )
			{
				skipLine();

				continue;
			}

			chr = content[cursor];

			if ( chr == '\0' )
				break;

			if ( chr == '\"' )
			{
				state = State::ParsingKey;
				cursor += 1;
				contexts.back().key_start = cursor;
			}
			else if ( chr == '}' )
			{
				if ( contexts.size() == 1 )
				{
					return std::make_pair(
						std::move( contexts.front().value ),
						ParsingError{ .position = cursor,
									  .message = "Unexpected '}'" } );
				}

				cursor += 1;

				auto prev = contexts.end();
				--prev;
				--prev;

				if ( prev->value.IsArray() )
				{
					state = State::SearchingValue;
					prev->value.GetArray().push_back( std::move( ctx.value ) );
				}
				else if ( prev->value.IsObject() )
				{
					ASSERT( !prev->key.empty() );

					state = State::SearchingKey;

					const auto key = std::string{ prev->key };
					auto& prev_object = prev->value.GetObject();

					auto dst_entry = prev_object.find( key );

					if ( dst_entry == prev_object.end() ||
						 ( !dst_entry->second.IsObject() ||
						   !ctx.value.IsObject() ) )
					{
						prev_object.insert_or_assign( key,
													  std::move( ctx.value ) );
					}
					else
					{
						Merge( dst_entry->second.GetObject(),
							   std::move( ctx.value ).IntoObject() );
					}
				}
				else
				{
					UNREACHABLE();
				}

				contexts.pop_back();
			}
			else
			{
				return std::make_pair(
					std::move( contexts.front().value ),
					ParsingError{ .position = cursor,
								  .message = "Expected '\"' character" } );
			}

			break;
		}
		case State::ParsingKey:
		{
			if ( chr == '\"' && content[cursor - 1] != '\\' )
			{
				state = State::SearchingValue;
				ctx.key =
					content.substr( ctx.key_start, cursor - ctx.key_start );
			}

			cursor += 1;

			break;
		}
		case State::SearchingValue:
		{
			if ( !skipSpaces() )
				break;

			if ( tryParseText( "//" ) )
			{
				skipLine();

				continue;
			}

			chr = content[cursor];

			if ( chr == '{' )
			{
				state = State::SearchingKey;
				contexts.emplace_back(
					Context{ .value = Value::EmptyObject() } );
				cursor += 1;
			}
			else if ( chr == '[' )
			{
				state = State::SearchingValue;
				contexts.emplace_back(
					Context{ .value = Value::EmptyArray() } );
				cursor += 1;
			}
			else if ( chr == ']' )
			{
				if ( contexts.size() == 1 )
				{
					return std::make_pair(
						std::move( contexts.front().value ),
						ParsingError{ .position = cursor,
									  .message = "Unexpected ']'" } );
				}

				cursor += 1;

				auto prev = contexts.end();
				--prev;
				--prev;

				if ( prev->value.IsArray() )
				{
					state = State::SearchingValue;
					prev->value.GetArray().push_back( std::move( ctx.value ) );
				}
				else if ( prev->value.IsObject() )
				{
					ASSERT( !prev->key.empty() );

					state = State::SearchingKey;
					prev->value.GetObject().insert_or_assign(
						std::string{ prev->key }, std::move( ctx.value ) );
				}

				contexts.pop_back();
			}
			else if ( chr == '\"' )
			{
				state = State::ParsingString;
				cursor += 1;
				ctx.value_start = cursor;
			}
			else if ( str::IsDigit( chr ) || chr == '.' )
			{
				state = State::ParsingNumber;
				ctx.value_start = cursor;
				cursor += 1;
			}
			else if ( tryParseText( "true" ) )
			{
				if ( ctx.value.IsArray() )
				{
					state = State::SearchingValue;
					ctx.value.GetArray().push_back( Value::FromBool( true ) );
				}
				else if ( ctx.value.IsObject() )
				{
					ASSERT( !ctx.key.empty() );

					state = State::SearchingKey;
					ctx.value.GetObject().insert_or_assign(
						std::string{ ctx.key }, Value::FromBool( true ) );
					ctx.key = std::string_view{};
				}
				else
				{
					UNREACHABLE();
				}
			}
			else if ( tryParseText( "false" ) )
			{
				if ( ctx.value.IsArray() )
				{
					state = State::SearchingValue;
					ctx.value.GetArray().push_back( Value::FromBool( false ) );
				}
				else if ( ctx.value.IsObject() )
				{
					ASSERT( !ctx.key.empty() );

					state = State::SearchingKey;
					ctx.value.GetObject().insert_or_assign(
						std::string{ ctx.key }, Value::FromBool( false ) );
					ctx.key = std::string_view{};
				}
				else
				{
					UNREACHABLE();
				}
			}
			else if ( tryParseText( "null" ) )
			{
				if ( !ctx.value.IsObject() )
				{
					return std::make_pair(
						std::move( contexts.front().value ),
						ParsingError{ .position = cursor,
									  .message = "Unexpected 'null'" } );
				}

				ASSERT( !ctx.key.empty() );

				state = State::SearchingKey;
				ctx.value.GetObject().erase( std::string{ ctx.key } );
			}
			else
			{
				return std::make_pair(
					std::move( contexts.front().value ),
					ParsingError{ .position = cursor,
								  .message = "Expected value" } );
			}

			break;
		}
		case State::ParsingString:
		{
			if ( chr == '\"' && content[cursor - 1] != '\\' )
			{
				const auto value =
					content.substr( ctx.value_start, cursor - ctx.value_start );

				if ( ctx.value.IsArray() )
				{
					state = State::SearchingValue;
					ctx.value.GetArray().emplace_back( value );
				}
				else if ( ctx.value.IsObject() )
				{
					ASSERT( !ctx.key.empty() );

					state = State::SearchingKey;
					ctx.value.GetObject().insert_or_assign(
						std::string{ ctx.key }, Value{ value } );
					ctx.key = std::string_view{};
				}
				else
				{
					UNREACHABLE();
				}
			}

			cursor += 1;

			break;
		}
		case State::ParsingNumber:
		{
			if ( str::IsDigit( chr ) || chr == '.' )
			{
				cursor += 1;

				continue;
			}

			const auto result = istd::str::FromChars<double>(
				content.substr( ctx.value_start, ctx.value_start - cursor ) );

			if ( !result.has_value() )
			{
				return std::make_pair(
					std::move( contexts.front().value ),
					ParsingError{ .position = cursor,
								  .message = "Invalid number format" } );
			}

			if ( ctx.value.IsArray() )
			{
				state = State::SearchingValue;
				ctx.value.GetArray().emplace_back( *result );
			}
			else if ( ctx.value.IsObject() )
			{
				ASSERT( !ctx.key.empty() );

				ctx.value.GetObject().insert_or_assign( std::string{ ctx.key },
														Value{ *result } );
				state = State::SearchingKey;
				ctx.key = std::string_view{};
			}
			else
			{
				UNREACHABLE();
			}

			break;
		}
		}
	}

	switch ( state )
	{
	case State::SearchingKey:
		break;
	case State::ParsingKey:
		return std::make_pair(
			std::move( contexts.front().value ),
			ParsingError{
				.position = cursor,
				.message = "Unexpected end of input while parsing key",
			} );
	case State::SearchingValue:
		return std::make_pair(
			std::move( contexts.front().value ),
			ParsingError{
				.position = cursor,
				.message = "Unexpected end of input while parsing value",
			} );
	case State::ParsingNumber:
		return std::make_pair(
			std::move( contexts.front().value ),
			ParsingError{
				.position = cursor,
				.message = "Unexpected end of input while parsing number value",
			} );
	case State::ParsingString:
		return std::make_pair(
			std::move( contexts.front().value ),
			ParsingError{
				.position = cursor,
				.message = "Unexpected end of input while parsing string value",
			} );
	}

	return std::make_pair( std::move( contexts.front().value ), std::nullopt );
}

constexpr std::string_view escapeSymbols = "\"";

void SerializeObjectInner( const Object&, a4::Object& ) noexcept;

void SerializeArrayInner( const Array& array, a4::Array& result ) noexcept
{

	for ( const auto& value : array )
	{
		if ( value.IsString() )
		{
			result.value.emplace_back( a4::String{ value.GetString() } );
		}
		else if ( value.IsNumber() )
		{
			result.value.emplace_back( a4::Number{ value.GetNumber() } );
		}
		else if ( value.IsBool() )
		{
			result.value.emplace_back( a4::Bool{ value.GetBool() } );
		}
		else if ( value.IsObject() )
		{
			a4::Object resultObject{};

			SerializeObjectInner( value.GetObject(), resultObject );

			result.value.emplace_back( std::move( resultObject ) );
		}
		else if ( value.IsArray() )
		{
			a4::Array resultArray{};

			SerializeArrayInner( value.GetArray(), resultArray );

			result.value.emplace_back( std::move( resultArray ) );
		}
		else
		{
			UNREACHABLE();
		}
	}
}

void SerializeObjectInner( const Object& object, a4::Object& result ) noexcept
{
	for ( const auto& [key, value] : object )
	{

		if ( value.IsString() )
		{
			result.value.insert_or_assign( std::string{ key },
										   a4::String{ value.GetString() } );
		}
		else if ( value.IsNumber() )
		{
			result.value.insert_or_assign( std::string{ key },
										   a4::Number{ value.GetNumber() } );
		}
		else if ( value.IsBool() )
		{
			result.value.insert_or_assign( std::string{ key },
										   a4::Bool{ value.GetBool() } );
		}
		else if ( value.IsObject() )
		{
			a4::Object resultObject{};

			SerializeObjectInner( value.GetObject(), resultObject );

			result.value.insert_or_assign( std::string{ key },
										   std::move( resultObject ) );
		}
		else if ( value.IsArray() )
		{
			a4::Array resultArray{};

			SerializeArrayInner( value.GetArray(), resultArray );

			result.value.insert_or_assign( std::string{ key },
										   std::move( resultArray ) );
		}
		else
		{
			UNREACHABLE();
		}
	}
}

export a4::Object Serialize( const Object& object ) noexcept
{
	a4::Object result{};

	SerializeObjectInner( object, result );

	return result;
}

void ToStringInner( const a4::Object&, isize, std::string& ) noexcept;

void ToStringArrayInner( const a4::Array& array, const isize indent,
						 std::string& result, const bool nested ) noexcept
{
	const auto& [arrayValue] = array;

	if ( arrayValue.empty() )
	{
		if ( nested )
			result += std::format( "{}[]\n", std::string( indent, ' ' ) );
		else
			result += "[]\n";

		return;
	}

	const bool horizontal =
		std::ranges::find_if(
			arrayValue,
			[]( const auto& value ) -> bool
			{
				return std::holds_alternative<a4::Object>( value ) ||
					   std::holds_alternative<a4::Array>( value );
			} ) == arrayValue.cend();

	if ( horizontal )
	{
		if ( nested )
			result += std::format( "{}[", std::string( indent, ' ' ) );
		else
			result += '[';
	}
	else
	{
		if ( nested )
			result += std::format( "{}[\n", std::string( indent, ' ' ) );
		else
			result += "[\n";
	}

	for ( auto it = arrayValue.cbegin(); it != arrayValue.cend(); ++it )
	{
		if ( std::holds_alternative<a4::String>( *it ) )
		{
			const auto& [stringValue] = std::get<a4::String>( *it );

			if ( horizontal )
			{
				result += std::format(
					"\"{}\"", str::Escape( stringValue, escapeSymbols ) );
			}
			else
			{
				result +=
					std::format( "{}\"{}\"\n", std::string( indent, ' ' ),
								 str::Escape( stringValue, escapeSymbols ) );
			}
		}
		else if ( std::holds_alternative<a4::Number>( *it ) )
		{
			const auto [numberValue] = std::get<a4::Number>( *it );

			if ( horizontal )
			{
				result += std::format( "{}", numberValue );
			}
			else
			{
				result += std::format( "{}{}\n", std::string( indent, ' ' ),
									   numberValue );
			}
		}
		else if ( std::holds_alternative<a4::Bool>( *it ) )
		{
			const auto [booleanValue] = std::get<a4::Bool>( *it );

			if ( horizontal )
			{
				result += std::format( "{}", booleanValue );
			}
			else
			{
				result += std::format( "{}{}\n", std::string( indent, ' ' ),
									   booleanValue );
			}
		}
		else if ( std::holds_alternative<a4::Array>( *it ) )
		{
			ToStringArrayInner( std::get<a4::Array>( *it ), indent, result,
								true );
		}
		else if ( std::holds_alternative<a4::Object>( *it ) )
		{
			result += std::format( "{}{{\n", std::string( indent, ' ' ) );

			ToStringInner( std::get<a4::Object>( *it ), indent + 2, result );

			result += std::format( "{}}}\n", std::string( indent, ' ' ) );
		}
		else
		{
			UNREACHABLE();
		}

		if ( horizontal && it != arrayValue.cend() - 1 )
		{
			result += ' ';
		}
	}

	result += "]\n";
}

void ToStringInner( const a4::Object& object, const isize indent,
					std::string& result ) noexcept
{
	for ( const auto& [key, value] : object.value )
	{
		const auto keyPart =
			std::format( "{}\"{}\"", std::string( indent, ' ' ),
						 str::Escape( key, escapeSymbols ) );

		if ( std::holds_alternative<a4::String>( value ) )
		{
			const auto& [stringValue] = std::get<a4::String>( value );

			result += std::format( "{} \"{}\"\n", keyPart,
								   str::Escape( stringValue, escapeSymbols ) );
		}
		else if ( std::holds_alternative<a4::Number>( value ) )
		{
			const auto [number_value] = std::get<a4::Number>( value );

			result += std::format( "{} {}\n", keyPart, number_value );
		}
		else if ( std::holds_alternative<a4::Bool>( value ) )
		{
			const auto [boolean_value] = std::get<a4::Bool>( value );

			result += std::format( "{} {}\n", keyPart, boolean_value );
		}
		else if ( std::holds_alternative<a4::Array>( value ) )
		{
			result += std::format( "{} ", keyPart );

			const auto& array = std::get<a4::Array>( value );

			ToStringArrayInner( array, indent + 2, result, false );
		}
		else if ( std::holds_alternative<a4::Object>( value ) )
		{
			result += std::format( "{} {{\n", keyPart );

			ToStringInner( std::get<a4::Object>( value ), indent + 2, result );

			result += std::format( "{}}}\n", std::string( indent, ' ' ) );
		}
	}
}

export std::string ToString( const a4::Object& object ) noexcept
{
	std::string result{};

	ToStringInner( object, 0, result );

	return result;
}

} // namespace istd::format::kv
