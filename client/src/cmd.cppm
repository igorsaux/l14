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
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <span>
#include <string>
#include <vector>

export module l14.client.cmd;

import :commands;
import :convars;
import :downloader;
import :server;

import istd.crash;
import istd.defer;
import istd.format.kv;
import istd.fs;
import istd.str;
import istd;
import l14.shared.args;
import l14.client.globals;
import l14.shared.client;
import l14.shared.config;
import l14.shared.constants;

namespace l14
{

export class L14Cmd final : public IL14Cmd
{
  public:
	void Quit() noexcept
	{
		m_quit = true;
	}

	[[nodiscard]] const Config& GetConfig() const noexcept
	{
		return m_config;
	}

	int Run( const int argc, char* argv[] ) noexcept override
	{
		ASSERT( argc >= 1 );

		m_exeFolder = argv[0];

		if ( m_exeFolder.is_relative() )
			m_exeFolder = std::filesystem::absolute( m_exeFolder );

		m_exeFolder = m_exeFolder.parent_path();

		ASSERT( !m_exeFolder.empty() );

		try
		{
			return RunInner( argc, argv );
		}
		catch ( const std::exception& err )
		{
			std::cerr << "Fatal error: " << err.what() << std::endl;

			return 1;
		}
	}

	[[nodiscard]] static CommandDef*
	FindCommand( const std::string_view name ) noexcept
	{
		auto ptr = CommandDef::List();

		while ( ptr != nullptr )
		{
			if ( ptr->Name() == name )
				return ptr;

			ptr = ptr->Next();
		}

		return nullptr;
	}

	[[nodiscard]] static ConVarDef*
	FindConVar( const std::string_view name ) noexcept
	{
		auto ptr = ConVarDef::List();

		while ( ptr != nullptr )
		{
			if ( ptr->Name() == name )
				return ptr;

			ptr = ptr->Next();
		}

		return nullptr;
	}

	[[nodiscard]] bool Execute( const std::string_view input,
								const bool multiline ) const noexcept
	{
		std::vector<std::vector<std::string>> lines{};

		const auto _lines = istd::str::Split( input, multiline ? "\r\n" : "+" );
		lines.reserve( _lines.size() );

		for ( const auto& line : _lines )
		{
			if ( line.starts_with( "//" ) )
				continue;

			std::vector<std::string> args{};

			// Split & convert into string
			{
				const auto _args = istd::str::SplitArgs( line );

				if ( _args.empty() )
					continue;

				args.reserve( _args.size() );

				for ( const auto& arg : _args )
					args.emplace_back( istd::str::UnEscape( arg, "\"" ) );
			}

			lines.push_back( std::move( args ) );
		}

		for ( const auto& line : lines )
		{
			if ( m_quit )
				return true;

			std::vector<std::string_view> args{};
			args.reserve( line.size() );

			for ( const auto& arg : line )
				args.emplace_back( arg );

			if ( args.empty() )
				return true;

			if ( args[0].starts_with( '@' ) )
			{
				const auto conVar = FindConVar( args[0].starts_with( "@@" )
													? args[0].substr( 2 )
													: args[0].substr( 1 ) );

				if ( conVar == nullptr )
				{
					std::cerr << "Unknown ConVar: " << args[0] << std::endl;

					if ( g_ignoreErrorsCVar == False )
						return false;

					continue;
				}

				// Set the new value
				if ( args.size() == 2 )
				{
					if ( !SetConVar( *conVar, args[1] ) &&
						 g_ignoreErrorsCVar == False )
					{
						return false;
					}

					continue;
				}

				// Print the current value
				if ( args.size() == 1 )
				{
					// Reset to the default value
					if ( args[0].starts_with( "@@" ) )
					{
						*conVar = conVar->DefaultValue();

						continue;
					}

					const auto& value = conVar->Value();

					std::cout << std::format( "{}", value ) << std::endl;

					continue;
				}

				std::cerr << "ConVar expects only one argument" << std::endl;

				if ( g_ignoreErrorsCVar == False )
					return false;
			}
			else
			{
				const auto command = FindCommand( args[0] );

				if ( command == nullptr )
				{
					std::cerr << "Unknown command: " << args[0] << std::endl;

					if ( g_ignoreErrorsCVar == False )
						return false;

					continue;
				}

				const auto ret =
					command->Invoke( std::span{ args }.subspan( 1 ) );

				if ( !ret && g_ignoreErrorsCVar == False )
					return false;
			}
		}

		return true;
	}

  private:
	int RunInner( const int argc, char* argv[] )
	{
		if ( !LoadConfig() )
			return 1;

		// Run commands from args
		{
			std::string input{};
			input.reserve( argc * 8 );

			for ( auto i = 1; i < argc; i++ )
				input += std::format( " \"{}\"", argv[i] );

			if ( !Execute( input, false ) && g_ignoreErrorsCVar == False )
				return 1;
		}

		if ( m_quit )
			return 0;

		if ( g_noTipsCVar == False )
		{
			const std::array<std::reference_wrapper<const CommandDef>, 4>
				tipCommands{ g_quitCommand, g_helpCommand,
							 g_commandsListCommand, g_conVarsListCommand };

			std::cerr << "Tips:\n";

			for ( const auto& cmd : tipCommands )
			{
				std::cerr << std::format( "\t'{}' - {}\n", cmd.get().Name(),
										  cmd.get().Desc() );
			}

			std::cerr << std::endl;
		}

		std::cerr << "L14 (C) 2025 Igor Spichkin\nVersion: " << k_version
				  << std::endl;

		// Interactive mode
		while ( !m_quit )
		{
			std::cerr << "$ ";

			std::string input{};

			if ( std::getline( std::cin, input ).eof() )
				return 0;

			std::ignore = Execute( input, true );
		}

		return 0;
	}

	bool LoadConfig()
	{
		const auto configFilePath =
			istd::fs::GetAbsolutePath( k_configFileName, m_exeFolder );

		// Create default
		if ( !istd::fs::Exists( configFilePath ) )
		{
			const auto configText =
				istd::format::kv::ToString( Config::Default().Serialize() );

			std::ofstream stream{ configFilePath.string() };
			stream.exceptions( std::ios::badbit );

			try
			{
				istd::fs::WriteAll( stream, configText );
			}
			catch ( std::ios::failure& err )
			{
				std::cerr << "Failed to write default config: " << err.what()
						  << std::endl;

				return g_ignoreErrorsCVar == True;
			}

			return true;
		}

		std::string configText;

		try
		{
			std::ifstream stream{ configFilePath.string() };
			stream.exceptions( std::ios::badbit );

			const auto content = istd::fs::ReadAll( stream );

			configText = std::string( content.data(), content.size() );
		}
		catch ( std::ios::failure& err )
		{
			std::cerr << "Failed to read the config file: " << err.what()
					  << std::endl;

			return g_ignoreErrorsCVar == True;
		}

		{
			const auto [config, error] = istd::format::kv::Parse( configText );

			if ( error.has_value() )
			{
				std::cerr << "Failed to parse the config file: "
						  << error.value().message << " at "
						  << error.value().position << std::endl;

				return g_ignoreErrorsCVar == True;
			}

			ASSERT( config.IsObject() );

			const auto serialized =
				istd::format::kv::Serialize( config.GetObject() );

			m_config = Config::Deserialize( serialized );
		}

		return true;
	}

	static bool SetConVar( ConVarDef& conVar,
						   const std::string_view newValue ) noexcept
	{
		const auto defaultValue = conVar.DefaultValue();

		if ( defaultValue.IsNumber() )
		{
			const auto value = istd::str::FromChars<double>( newValue );

			if ( !value.has_value() )
			{
				std::cerr << std::format( "Can't convert '{}' into a number",
										  newValue )
						  << std::endl;

				return false;
			}

			conVar = *value;
		}
		else if ( defaultValue.IsBool() )
		{
			if ( newValue == "0" || newValue == "false" )
				conVar = False;
			else if ( newValue == "1" || newValue == "true" )
				conVar = True;
			else
			{
				std::cerr << std::format( "Can't convert '{}' into a boolean",
										  newValue )
						  << std::endl;

				return false;
			}
		}
		else if ( defaultValue.IsString() )
			conVar = std::string{ newValue };

		return true;
	}

	std::filesystem::path m_exeFolder{};
	Config m_config{};
	bool m_quit{};
};

//-----------------------------------------------------------------------------
// Commands
//-----------------------------------------------------------------------------

bool QuitCommandCb( const CommandDef&,
					const std::span<const std::string_view> ) noexcept
{
	const auto cmd = dynamic_cast<L14Cmd*>( g_cmd );

	ASSERT( cmd != nullptr );

	cmd->Quit();

	return true;
}

bool EchoCommandCb( const CommandDef&,
					const std::span<const std::string_view> args ) noexcept
{
	for ( size_t i = 0; i < args.size(); i++ )
	{
		std::cerr << args[i];

		if ( i != args.size() - 1 )
			std::cerr << ' ';
	}

	std::cerr << std::endl;

	return true;
}

bool ExecCommandCb( const CommandDef& def,
					const std::span<const std::string_view> args )
{
	static bool s_guard{};

	if ( s_guard )
	{
		std::cerr << std::format( "Command '{}' is not allowed to be nested",
								  def.Name() )
				  << std::endl;

		return false;
	}

	if ( args.size() != 1 )
	{
		std::cerr << std::format( "Command '{}' expects one argument",
								  def.Name() )
				  << std::endl;

		return false;
	}

	s_guard = true;

	const auto _ = istd::Defer(
		[]
		{
			s_guard = false;
		} );

	const auto path = std::filesystem::path{ args[0] };

	if ( !path.is_relative() )
	{
		std::cerr << std::format( "Path '{}' is not relative", path.string() )
				  << std::endl;

		return false;
	}

	if ( !istd::fs::Exists( path ) )
	{
		std::cerr << std::format( "File '{}' does not exist", path.string() )
				  << std::endl;

		return false;
	}

	std::ifstream stream{ path };
	stream.exceptions( std::ios::badbit );

	std::vector<char> content;

	try
	{
		content = istd::fs::ReadAll( stream );
	}
	catch ( const std::ios::failure& err )
	{
		std::cerr << "File reading error: " << err.what() << std::endl;

		return false;
	}

	const auto cmd = dynamic_cast<L14Cmd*>( g_cmd );

	ASSERT( cmd != nullptr );

	return cmd->Execute( std::string_view{ content.data(), content.size() },
						 true );
}

bool HelpCommandCb( const CommandDef& def,
					const std::span<const std::string_view> args ) noexcept
{
	if ( args.size() != 1 )
	{
		std::cerr << std::format( "Command '{}' expects one argument",
								  def.Name() )
				  << std::endl;

		return false;
	}

	if ( args[0].starts_with( '@' ) )
	{
		const auto conVar = L14Cmd::FindConVar( args[0].substr( 1 ) );

		if ( conVar == nullptr )
		{
			std::cerr << std::format( "ConVar '{}' not found", args[0] )
					  << std::endl;

			return false;
		}

		std::cerr << std::format( "@{} - {}\nDefault: '{}'", conVar->Name(),
								  conVar->Desc(), conVar->DefaultValue() )
				  << std::endl;
	}
	else
	{
		const auto command = L14Cmd::FindCommand( args[0] );

		if ( command == nullptr )
		{
			std::cerr << std::format( "Command '{}' not found", args[0] )
					  << std::endl;

			return false;
		}

		std::cerr << std::format( "{} - {}\nUsage:\n\t{} {}", command->Name(),
								  command->Desc(), command->Name(),
								  command->Usage() )
				  << std::endl;
	}

	return true;
}

bool CommandsListCommandCb( const CommandDef&,
							std::span<const std::string_view> ) noexcept
{
	auto ptr = CommandDef::List();

	while ( ptr != nullptr )
	{
		std::cerr << std::format( "'{}' - {}\n", ptr->Name(), ptr->Desc() );

		ptr = ptr->Next();
	}

	std::cerr << std::endl;

	return true;
}

bool ConVarsListCommandCb( const CommandDef&,
						   std::span<const std::string_view> ) noexcept
{
	auto ptr = ConVarDef::List();

	while ( ptr != nullptr )
	{
		std::cerr << std::format( "'@{}' - {}\n", ptr->Name(), ptr->Desc() );

		ptr = ptr->Next();
	}

	std::cerr << std::endl;

	return true;
}

} // namespace l14
