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

export module l14.client.cmd:convars;

export import l14.client.convar;

import istd;

namespace l14
{

const ConVarDef
	g_ignoreErrorsCVar( "IgnoreErrors", False,
						"continue execution even if an error occurs" );

const ConVarDef g_serversEndpointCVar( "ServersEndpoint", "api/servers" );

const ConVarDef g_serverInfoEndpointCVar( "ServerInfoEndpoint", "info" );

const ConVarDef g_httpTimeoutCVar( "HttpTimeout", 10000,
								   "time in milliseconds" );

const ConVarDef g_noTipsCVar( "NoTips", False, "do not print tips on start" );

const ConVarDef g_clientsFolderCVar( "ClientsFolder", "./clients/",
									 "where to download the clients" );

const ConVarDef
	g_strictHashCVar( "StrictHash", True,
					  "error occurs when the hashes are incorrect" );

const ConVarDef g_cacheDirCVar( "CacheDir", "./cache/",
								"the directory for cached files" );

} // namespace l14
