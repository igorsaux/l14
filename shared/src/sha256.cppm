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
#include <bit>
#include <format>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <span>
#include <string>
#include <utility>
#include <vector>

export module l14.shared.sha256;

import istd;
import istd.crash;
import istd.hasher;

namespace l14
{

export class Sha256 final : public istd::IHasher
{
  public:
	Sha256()
	{
		m_ctx = EVP_MD_CTX_new();

		if ( m_ctx == nullptr )
			throw istd::HashException{ GetError() };

		if ( EVP_DigestInit( m_ctx, EVP_sha256() ) == 0 )
			throw istd::HashException{ GetError() };
	}

	Sha256( const Sha256& ) = delete;
	Sha256& operator=( const Sha256& ) = delete;

	Sha256( Sha256&& other ) noexcept
		: m_ctx( std::exchange( m_ctx, other.m_ctx ) )
	{
	}

	Sha256& operator=( Sha256&& other ) noexcept
	{
		std::swap( m_ctx, other.m_ctx );

		return *this;
	}

	bool operator==( const std::nullptr_t& ) const
	{
		return m_ctx == nullptr;
	}

	[[nodiscard]] static std::string
	ToString( const std::span<const char> bytes ) noexcept
	{
		std::string string{};
		string.reserve( bytes.size() );

		for ( const auto chr : bytes )
			string += std::format( "{:0>2x}", chr );

		return string;
	}

	void Update( const std::span<const char> input ) override
	{
		ASSERT( m_ctx != nullptr );

		if ( EVP_DigestUpdate( m_ctx, input.data(), input.size() ) == 0 )
		{
			throw istd::HashException{ GetError() };
		}
	}

	[[nodiscard]] std::vector<char> Final() noexcept override
	{
		unsigned int size{};
		std::vector<char> result{};
		result.resize( EVP_MD_size( EVP_sha256() ) );

		EVP_DigestFinal( m_ctx, std::bit_cast<u8*>( result.data() ), &size );

		result.resize( size );

		return result;
	}

	~Sha256() override
	{
		if ( m_ctx == nullptr )
			return;

		EVP_MD_CTX_free( std::exchange( m_ctx, nullptr ) );
	}

  private:
	[[nodiscard]] static std::string GetError() noexcept
	{
		const auto err = ERR_get_error();

		return std::format( "{}: {}", ERR_lib_error_string( err ),
							ERR_reason_error_string( err ) );
	}

	EVP_MD_CTX* m_ctx{};
};

} // namespace l14
