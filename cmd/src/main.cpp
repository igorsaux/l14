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

#include "istd/assert.hpp"
#include <filesystem>
#include <iostream>

import istd.crash;
import istd.dll;
import istd.fs;
import l14.cmd.process;
import l14.cmd.updater;
import l14.shared.app;
import l14.shared.args;
import l14.shared.client;
import l14.shared.constants;

using namespace l14;

bool Update( const int argc, char* argv[] ) noexcept
{
	try
	{
		std::filesystem::path exePath{ argv[0] };

		if ( exePath.is_relative() )
			exePath = std::filesystem::absolute( exePath );

		std::cerr << "Checking for updates" << std::endl;

		const auto update = updater::RunAutoUpdate( exePath );

		if ( update.needsRestart )
		{
			RestartProcess( argc, argv );

			return true;
		}
	}
	catch ( const std::exception& err )
	{
		std::cerr << "Fatal error: " << err.what() << std::endl;

		return false;
	}

	return false;
}

int main( const int argc, char* argv[] )
{
	bool doUpdate = true;
	int argsSub = -1;

	for ( auto i = 1; i < argc; i++ )
	{
		const std::string_view arg{ argv[i] };

		if ( argsSub == -1 && arg.starts_with( '/' ) )
			argsSub = argc - i;

		if ( arg == "/noupdate" )
			doUpdate = false;
	}

	if ( argsSub == -1 )
		argsSub = 0;

	std::filesystem::path exePath{ argv[0] };

	if ( exePath.is_relative() )
		exePath = std::filesystem::absolute( exePath );

	if ( doUpdate && Update( argc, argv ) )
		return 0;

	auto clientDllPath = exePath.parent_path() / k_clientDll;

	const auto [error, clientDll] = istd::IDll::Open( clientDllPath );

	if ( clientDll == nullptr )
	{
		std::cerr << "Failed to load " << k_clientDll << ": " << error
				  << std::endl;

		return 1;
	}

	const auto client = IApp::Load<IL14Client>( *clientDll );

	if ( client == nullptr )
	{
		std::cerr << "Invalid " << k_clientDll << std::endl;

		return 1;
	}

	const auto cmd = client->CreateCmd();

	ASSERT( cmd != nullptr );

	const auto ret = cmd->Run( argc - argsSub, argv );

	client->DestroyCmd();

	return ret;
}
