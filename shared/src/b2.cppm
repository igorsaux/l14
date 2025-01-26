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
#include <blake2.h>
#include <format>
#include <span>
#include <string>
#include <vector>

export module l14.shared.b2;

import istd.crash;
import istd.hasher;
import istd;

namespace l14
{

export class Blake2B final : public istd::IHasher
{
  public:
	Blake2B( const size_t length = BLAKE2B_OUTBYTES ) : m_length( length )
	{
		ASSERT( blake2b_init( &m_state, m_length ) == 0 );
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

	void Update( const std::span<const char> input ) noexcept override
	{
		ASSERT( blake2b_update( &m_state, std::bit_cast<u8*>( input.data() ),
								input.size() ) == 0 );
	}

	[[nodiscard]] std::vector<char> Final() noexcept override
	{
		std::vector<char> result{};
		result.resize( m_length, '\0' );

		ASSERT( blake2b_final( &m_state, std::bit_cast<u8*>( result.data() ),
							   result.size() ) == 0 );

		return result;
	}

  private:
	blake2b_state m_state{};
	size_t m_length{};
};

} // namespace l14
