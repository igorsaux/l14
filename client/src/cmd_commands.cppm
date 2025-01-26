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

#include <span>
#include <string_view>

export module l14.client.cmd:commands;

export import l14.client.command;

namespace l14
{

bool QuitCommandCb( const CommandDef&,
					std::span<const std::string_view> ) noexcept;

export const CommandDef g_quitCommand( "quit", QuitCommandCb, "quits the cmd" );

bool EchoCommandCb( const CommandDef&,
					std::span<const std::string_view> ) noexcept;

export const CommandDef
	g_echoCommand( "echo", EchoCommandCb,
				   "prints the given arguments to the console", "[ARG...]" );

bool ExecCommandCb( const CommandDef&, std::span<const std::string_view> );

export const CommandDef
	g_execCommand( "exec", ExecCommandCb,
				   "execs the specified file, the path must be relative",
				   "<PATH>" );

bool HelpCommandCb( const CommandDef&,
					std::span<const std::string_view> ) noexcept;

export const CommandDef
	g_helpCommand( "help", HelpCommandCb,
				   "prints the help text for the specified command or convar.",
				   "<COMMAND>" );

bool CommandsListCommandCb( const CommandDef&,
							std::span<const std::string_view> ) noexcept;

export const CommandDef
	g_commandsListCommand( "commands_list", CommandsListCommandCb,
						   "prints the list of all the commands" );

bool ConVarsListCommandCb( const CommandDef&,
						   std::span<const std::string_view> ) noexcept;

export const CommandDef
	g_conVarsListCommand( "convars_list", ConVarsListCommandCb,
						  "print the list of all the convars" );

namespace sv
{

bool FetchServersCommandCb( const CommandDef&,
							std::span<const std::string_view> ) noexcept;

}

export const CommandDef g_fetchServersCommand(
	"fetch_servers", sv::FetchServersCommandCb,
	"fetches the server list from the hub and prints it in JSON format",
	"<HUB_URI>" );

namespace sv
{

bool ServerInfoCommandCb( const CommandDef&,
						  std::span<const std::string_view> ) noexcept;

}

export const CommandDef g_serverInfoCommand( "server_info",
											 sv::ServerInfoCommandCb,
											 "fetches the server info",
											 "<SERVER_URI>" );

namespace sv
{

bool ManifestDownloadProtoCommandCb(
	const CommandDef&, std::span<const std::string_view> ) noexcept;

}

export const CommandDef g_manifestDownloadProtoCommand(
	"manifest_download_proto", sv::ManifestDownloadProtoCommandCb,
	"fetches the manifest download protocol versions", "<SERVER_URI>" );

namespace ld
{

bool DownloadClientCommandCb( const CommandDef&,
							  std::span<const std::string_view> );

}

export const CommandDef g_downloadClientCommand(
	"download_client", ld::DownloadClientCommandCb,
	"just downloads the client of the specified server. Available methods: 0 - "
	"auto, 1 - manifest, 2 - zip",
	"<SERVER_URI> <PATH> [-method NUMBER] [-overwrite BOOL]" );

} // namespace l14
