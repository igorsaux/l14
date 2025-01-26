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

#include <filesystem>
#include <iostream>
#include <vector>

import istd.fs;
import l14.shared.b2;

int main( const int argc, const char* argv[] )
{
	if ( argc != 2 )
	{
		std::cerr << "Pass the path to the file" << std::endl;

		return 1;
	}

	const std::filesystem::path filePath{ argv[1] };

	std::vector<char> hashBytes;

	{
		l14::Blake2B b2{};
		hashBytes = istd::fs::ComputeHash( filePath, b2 );
	}

	const auto hash = l14::Blake2B::ToString( hashBytes );

	std::cout << hash << std::endl;

	return 0;
}
