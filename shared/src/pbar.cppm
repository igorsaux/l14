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

#include <algorithm>
#include <iostream>

export module l14.shared.pbar;

import istd;

namespace l14
{

export class ProgressBar
{
  public:
	ProgressBar() = default;

	ProgressBar( const i8 width ) : m_width( width )
	{
	}

	void Set( float progress ) noexcept
	{
		progress = std::clamp( progress, 0.0f, 1.0f );

		if ( progress == 1.0f )
		{
			End();

			return;
		}

		auto pos = static_cast<isize>(
			progress * ( static_cast<float>( m_width ) * 10.0f ) );
		pos = std::clamp( pos, m_lastPos, static_cast<isize>( m_width * 10 ) );

		if ( pos == m_lastPos )
			return;

		for ( isize i = m_lastPos + 1; i <= pos; i++ )
		{
			if ( i % m_width == 0 )
				std::cerr << i / m_width;
			else
				std::cerr << ".";
		}

		m_lastPos = pos;
	}

	void End() noexcept
	{
		if ( m_lastPos == -1 )
			return;

		std::cerr << 10 << std::endl;
		m_lastPos = -1;
	}

  private:
	i8 m_width = 4;
	isize m_lastPos = -1;
};

} // namespace l14
