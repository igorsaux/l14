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

#include <format>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

export module istd.uri;

import istd.str;
import istd;

namespace istd
{

std::regex k_uriRegex{
	R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
	std::regex_constants::extended,
};

export class Uri final
{
  public:
	Uri() = default;

	Uri( const Uri& base, const std::string_view relative )
	{
		*this = base;

		const auto paths = str::Split( relative, "/" );

		for ( const auto path : paths )
			m_path.emplace_back( path );
	}

	[[nodiscard]] static std::optional<Uri>
	Parse( const std::string& input ) noexcept
	{
		std::smatch smatch{};

		if ( !std::regex_match( input, smatch, k_uriRegex ) )
			return std::nullopt;

		Uri uri{};

		if ( smatch.size() < 4 )
			return std::nullopt;

		uri.m_strSize = input.size();
		uri.m_scheme = smatch[2];

		{
			const auto authority = smatch[4].str();
			const auto parts = str::Split( authority, ":" );

			if ( parts.size() == 2 )
			{
				uri.m_authority = parts[0];
				uri.m_port = parts[1];
			}
			else
			{
				uri.m_authority = authority;
			}
		}

		if ( smatch.size() >= 5 )
		{
			const auto segments = str::Split( smatch[5].str(), "/" );

			for ( const auto& segment : segments )
			{
				if ( segment.empty() )
					continue;

				uri.m_path.emplace_back( segment );
			}
		}

		if ( smatch.size() >= 7 )
		{
			const auto match = smatch[7].str();
			const auto params = str::Split( match, "&" );

			for ( const auto& param : params )
			{
				const auto pair = str::Split( param, "=" );

				if ( pair.size() != 2 )
					continue;

				uri.m_query.emplace_back( std::string{ pair[0] },
										  std::string{ pair[1] } );
			}
		}

		if ( smatch.size() >= 9 )
			uri.m_fragment = smatch[9];

		return uri;
	}

	[[nodiscard]] const std::string& Scheme() const noexcept
	{
		return m_scheme;
	}

	void Scheme( std::string scheme ) noexcept
	{
		m_scheme = std::move( scheme );
	}

	[[nodiscard]] const std::string& Authority() const noexcept
	{
		return m_authority;
	}

	void Authority( std::string authority ) noexcept
	{
		m_authority = std::move( authority );
	}

	[[nodiscard]] const std::string& Port() const noexcept
	{
		return m_port;
	}

	void Port( std::string port ) noexcept
	{
		m_port = std::move( port );
	}

	[[nodiscard]] const std::vector<std::string>& Path() const noexcept
	{
		return m_path;
	}

	[[nodiscard]] std::vector<std::string>& Path() noexcept
	{
		return m_path;
	}

	[[nodiscard]] const std::vector<std::pair<std::string, std::string>>&
	Query() const noexcept
	{
		return m_query;
	}

	[[nodiscard]] std::vector<std::pair<std::string, std::string>>&
	Query() noexcept
	{
		return m_query;
	}

	[[nodiscard]] const std::string& Fragment() const noexcept
	{
		return m_fragment;
	}

	void Fragment( std::string fragment ) noexcept
	{
		m_fragment = std::move( fragment );
	}

	[[nodiscard]] std::string ToString() const noexcept
	{
		std::string result{};

		if ( m_strSize != 0 )
			result.reserve( m_strSize );

		result =
			std::format( "{}://{}{}/", m_scheme, m_authority,
						 m_port.empty() ? "" : std::format( ":{}", m_port ) );

		for ( size_t i = 0; i < m_path.size(); i++ )
		{
			result += std::format( "{}{}", m_path[i],
								   i + 1 == m_path.size() ? "" : "/" );
		}

		if ( !m_query.empty() )
			result += '?';

		for ( size_t i = 0; i < m_query.size(); i++ )
		{
			const auto& [key, value] = m_query[i];

			result += std::format( "{}={}{}", key, value,
								   i + 1 == m_query.size() ? "" : "&" );
		}

		if ( !m_fragment.empty() )
			result += std::format( "#{}", m_fragment );

		return result;
	}

	[[nodiscard]] static std::string
	Escape( const std::string_view input ) noexcept
	{
		std::string result{};
		result.reserve( input.size() );

		for ( const auto chr : input )
		{
			switch ( chr )
			{
			case '!':
				result += "%21";

				break;
			case '#':
				result += "%23";

				break;
			case '$':
				result += "%24";

				break;
			case '&':
				result += "%26";

				break;
			case '\'':
				result += "%27";

				break;
			case '(':
				result += "%28";

				break;
			case ')':
				result += "%29";

				break;
			case '*':
				result += "%2A";

				break;
			case '+':
				result += "%2B";

				break;
			case ',':
				result += "%2C";

				break;
			case '/':
				result += "%2F";

				break;
			case ':':
				result += "%3A";

				break;
			case ';':
				result += "%3B";

				break;
			case '=':
				result += "%3D";

				break;
			case '?':
				result += "%3F";

				break;
			case '@':
				result += "%40";

				break;
			case '[':
				result += "%5B";

				break;
			case ']':
				result += "%5D";

				break;
			default:
				result += chr;
			}
		}

		return result;
	}

  private:
	usize m_strSize{};
	std::string m_scheme{};
	std::string m_authority{};
	std::string m_port{};
	std::vector<std::string> m_path{};
	std::vector<std::pair<std::string, std::string>> m_query{};
	std::string m_fragment{};
};

} // namespace istd

export template <>
struct std::formatter<istd::Uri, char>
{
	template <class ParseContext>
	constexpr typename ParseContext::iterator parse( ParseContext& ctx )
	{
		return ctx.begin();
	}

	template <class FmtContext>
	typename FmtContext::iterator format( const istd::Uri& uri,
										  FmtContext& ctx ) const
	{
		return std::format_to( ctx.out(), "{}", uri.ToString() );
	}
};
