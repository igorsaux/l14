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
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <span>
#include <string>
#include <string_view>

export module l14.client.cmd:server;

//-----------------------------------------------------------------------------
// Place here the functions that mainly do HTTP requests and interact with
// server API.
//-----------------------------------------------------------------------------

import :convars;

import istd;
import istd.str;
import istd.uri;
import l14.client.command;
import l14.shared.constants;
import l14.shared.http;

namespace l14::sv
{

bool FetchServersCommandCb(
	const CommandDef& dev,
	const std::span<const std::string_view> args ) noexcept
{
	if ( args.empty() )
	{
		std::cerr << std::format( "Command '{}' expects 1 argument",
								  dev.Name() )
				  << std::endl;

		return false;
	}

	istd::Uri hubUri;

	{
		auto _hubUrl = istd::Uri::Parse( std::string{ args[0] } );

		if ( !_hubUrl.has_value() )
		{
			std::cerr << "The hub uri is invalid" << std::endl;

			return false;
		}

		hubUri = std::move( _hubUrl.value() );
	}

	hubUri.Path().emplace_back( g_serversEndpointCVar.Value().GetString() );

	const RequestOptions options{
		.timeoutMs = static_cast<long>( g_httpTimeoutCVar.Value().GetNumber() ),
	};

	const auto& http = HttpClient::Instance();
	const auto response = http.Get( hubUri.ToString(), options );

	if ( !response.error.empty() )
	{
		std::cerr << std::format( "The request to '{}' returned error: {}",
								  hubUri.ToString(), response.error )
				  << std::endl;

		return false;
	}

	try
	{
		const auto json = nlohmann::json::parse( response.content.begin(),
												 response.content.end() );

		std::cout << json.dump() << std::endl;

		return true;
	}
	catch ( const nlohmann::json::parse_error& err )
	{
		std::cerr << std::format( "'{}' returned invalid json: {}",
								  hubUri.ToString(), err.what() )
				  << std::endl;

		return false;
	}
}

RequestResponse ServerInfoRequest( const istd::Uri& serverUri ) noexcept
{
	const auto infoEndpointUri =
		istd::Uri{ serverUri, g_serverInfoEndpointCVar.Value().GetString() };

	const RequestOptions options{
		.timeoutMs = static_cast<long>( g_httpTimeoutCVar.Value().GetNumber() ),
	};

	const auto& http = HttpClient::Instance();

	return http.Get( infoEndpointUri.ToString(), options );
}

[[nodiscard]] bool ToHttpUri( const istd::Uri& serverUri, istd::Uri& httpOut )
{
	httpOut = serverUri;

	if ( httpOut.Scheme() == "ss14" )
		httpOut.Scheme( "http" );
	else if ( httpOut.Scheme() == "ss14s" )
		httpOut.Scheme( "https" );
	else if ( httpOut.Scheme() != "http" && httpOut.Scheme() != "https" )
		return false;

	if ( httpOut.Port().empty() )
	{
		httpOut.Port( std::format( "{}", httpOut.Scheme() == "http"
											 ? k_httpDefaultServerPort
											 : k_httpsDefaultServerPort ) );
	}

	return true;
}

bool ServerInfoCommandCb(
	const CommandDef& def,
	const std::span<const std::string_view> args ) noexcept
{
	if ( args.size() != 1 )
	{
		std::cerr << std::format( "Command '{}' expects 1 arguments",
								  def.Name() )
				  << std::endl;

		return false;
	}

	istd::Uri serverUri;

	{
		auto _serverUri = istd::Uri::Parse( std::string{ args[0] } );

		if ( !_serverUri.has_value() )
		{
			std::cerr << "Invalid server uri: " << args[0] << std::endl;

			return false;
		}

		serverUri = std::move( _serverUri.value() );
	}

	istd::Uri httpServerUri;

	if ( !ToHttpUri( serverUri, httpServerUri ) )
	{
		std::cerr << "Invalid server uri: " << serverUri.ToString()
				  << std::endl;

		return false;
	}

	auto response = ServerInfoRequest( httpServerUri );

	if ( !response.error.empty() )
	{
		std::cerr << std::format( "The request returned error: {}",
								  response.error )
				  << std::endl;

		return false;
	}

	if ( response.code != 200 )
	{
		std::cerr << std::format( "The request returned code: {}",
								  response.code )
				  << std::endl;

		return false;
	}

	try
	{
		const auto object =
			nlohmann::json::parse( std::move( response.content ) );

		std::cout << object.dump() << std::endl;
	}
	catch ( const nlohmann::json::exception& err )
	{
		std::cerr << "The request returned invalid json: " << err.what()
				  << std::endl;

		return false;
	}

	return true;
}

struct ServerBuildInfo final
{
	[[nodiscard]] bool HasManifestDownloadingMethod() const noexcept
	{
		return !manifestHash.empty() && !manifestUrl.empty() &&
			   !manifestDownloadUrl.empty();
	}

	[[nodiscard]] bool HasZipDownloadingMethod() const noexcept
	{
		return !downloadUrl.empty();
	}

	[[nodiscard]] static ServerBuildInfo
	Parse( const nlohmann::basic_json<>& data ) noexcept
	{
		ServerBuildInfo info{};

		// Hash
		{
			const auto hash = data.find( "hash" );

			if ( hash != data.end() && hash->is_string() )
				info.hash = *hash;
		}

		// Fork id
		{
			const auto forkId = data.find( "fork_id" );

			if ( forkId != data.end() && forkId->is_string() )
				info.forkId = *forkId;
		}

		// Download uri
		{
			const auto downloadUrl = data.find( "download_url" );

			if ( downloadUrl != data.end() && downloadUrl->is_string() )
				info.downloadUrl = *downloadUrl;
		}

		// Manifest uri
		{
			const auto manifestUrl = data.find( "manifest_url" );

			if ( manifestUrl != data.end() && manifestUrl->is_string() )
				info.manifestUrl = *manifestUrl;
		}

		// Manifest hash
		{
			const auto manifestHash = data.find( "manifest_hash" );

			if ( manifestHash != data.end() && manifestHash->is_string() )
				info.manifestHash = *manifestHash;
		}

		// Manifest download uri
		{
			const auto manifestDownloadUrl =
				data.find( "manifest_download_url" );

			if ( manifestDownloadUrl != data.end() &&
				 manifestDownloadUrl->is_string() )
			{
				info.manifestDownloadUrl = *manifestDownloadUrl;
			}
		}

		// Acz
		{
			const auto acz = data.find( "acz" );

			if ( acz != data.end() && acz->is_boolean() )
				info.acz = *acz;
		}

		return info;
	}

	std::string hash{};
	std::string forkId{};
	std::string downloadUrl{};
	std::string manifestUrl{};
	std::string manifestHash{};
	std::string manifestDownloadUrl{};
	bool acz{};
};

struct ServerInfo final
{
	[[nodiscard]] static std::optional<ServerInfo>
	Parse( const RequestResponse& response ) noexcept
	{
		if ( !response.error.empty() )
		{
			std::cerr << std::format( "The request returned error: {}",
									  response.error )
					  << std::endl;

			return std::nullopt;
		}

		if ( response.code != 200 )
		{
			std::cerr << std::format( "The request returned code: {}",
									  response.code )
					  << std::endl;

			return std::nullopt;
		}

		nlohmann::basic_json<> data;

		try
		{
			data = nlohmann::json::parse( response.content );
		}
		catch ( const nlohmann::json::parse_error& err )
		{
			std::cerr << std::format( "Response returned invalid json: {}",
									  err.what() )
					  << std::endl;

			return std::nullopt;
		}

		if ( !data.is_object() )
		{
			std::cerr << "Response returned invalid json" << std::endl;

			return std::nullopt;
		}

		ServerInfo info{};

		// Build
		{
			const auto buildObject = data.find( "build" );

			if ( buildObject != data.end() && buildObject->is_object() )
			{
				info.build = ServerBuildInfo::Parse( *buildObject );
			}
		}

		return info;
	}

	ServerBuildInfo build{};
};

struct ManifestDownloadProtocol final
{
	[[nodiscard]] static std::optional<ManifestDownloadProtocol>
	Parse( const RequestResponse& response ) noexcept
	{
		if ( !response.error.empty() )
		{
			std::cerr << std::format( "The request returned error: {}",
									  response.error )
					  << std::endl;

			return std::nullopt;
		}

		if ( response.code != 204 )
		{
			std::cerr << std::format( "The request returned code: {}",
									  response.code )
					  << std::endl;

			return std::nullopt;
		}

		if ( response.headers.empty() )
			return std::nullopt;

		const auto minVersionHeader =
			response.headers.find( k_downloadMinProtocolHeader );

		if ( minVersionHeader == response.headers.end() )
		{
			std::cerr << std::format( "Response missing '{}' header",
									  k_downloadMinProtocolHeader )
					  << std::endl;

			return std::nullopt;
		}

		const auto maxVersionHeader =
			response.headers.find( k_downloadMaxProtocolHeader );

		if ( maxVersionHeader == response.headers.end() )
		{
			std::cerr << std::format( "Response missing '{}' header",
									  k_downloadMaxProtocolHeader )
					  << std::endl;

			return std::nullopt;
		}

		const auto minVersion =
			istd::str::FromChars<usize>( minVersionHeader->second );

		if ( !minVersion.has_value() )
		{
			std::cerr << std::format( "Response has invalid '{}' header value",
									  k_downloadMinProtocolHeader )
					  << std::endl;

			return std::nullopt;
		}

		const auto maxVersion =
			istd::str::FromChars<usize>( maxVersionHeader->second );

		if ( !maxVersion.has_value() )
		{
			std::cerr << std::format( "Response has invalid '{}' header value",
									  k_downloadMaxProtocolHeader )
					  << std::endl;

			return std::nullopt;
		}

		return ManifestDownloadProtocol{
			.minVersion = minVersion.value(),
			.maxVersion = maxVersion.value(),
		};
	}

	usize minVersion{};
	usize maxVersion{};
};

export std::optional<ManifestDownloadProtocol>
FetchManifestDownloadProtocol( const std::string& manifestDownloadUrl ) noexcept
{
	const RequestOptions options{
		.timeoutMs = static_cast<long>( g_httpTimeoutCVar.Value().GetNumber() ),
	};

	const auto& http = HttpClient::Instance();
	const auto response = http.Options( manifestDownloadUrl, options );

	return ManifestDownloadProtocol::Parse( response );
}

bool ManifestDownloadProtoCommandCb(
	const CommandDef& def,
	const std::span<const std::string_view> args ) noexcept
{
	if ( args.empty() )
	{
		std::cerr << std::format( "Command '{}' expects 1 argument",
								  def.Name() );

		return false;
	}

	istd::Uri serverUri;

	{
		auto _serverUri = istd::Uri::Parse( std::string{ args[0] } );

		if ( !_serverUri.has_value() )
		{
			std::cerr << "Invalid server uri: " << args[0] << std::endl;

			return false;
		}

		serverUri = std::move( _serverUri.value() );
	}

	istd::Uri serverHttpUri;

	if ( !ToHttpUri( serverUri, serverHttpUri ) )
	{
		std::cerr << "Invalid server uri: " << serverUri.ToString()
				  << std::endl;

		return false;
	}

	ServerInfo serverInfo;

	{
		const auto response = ServerInfoRequest( serverHttpUri );
		auto info = ServerInfo::Parse( response );

		if ( !info.has_value() )
		{
			std::cerr << "Failed to get the server info" << std::endl;

			return false;
		}

		serverInfo = std::move( info.value() );
	}

	const auto& buildInfo = serverInfo.build;

	if ( buildInfo.manifestDownloadUrl.empty() )
	{
		std::cerr << "Server does not have the manifest download uri"
				  << std::endl;

		return false;
	}

	const auto proto =
		FetchManifestDownloadProtocol( buildInfo.manifestDownloadUrl );

	if ( !proto.has_value() )
	{
		std::cerr << "Failed to get the manifest download protocol"
				  << std::endl;

		return false;
	}

	std::cout << std::format( "[{}, {}]", proto->minVersion, proto->maxVersion )
			  << std::endl;

	return true;
}

} // namespace l14::sv
