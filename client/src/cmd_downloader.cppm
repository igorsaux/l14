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
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <span>
#include <string_view>
#include <variant>

export module l14.client.cmd:downloader;

//-----------------------------------------------------------------------------
// Place here the functions that download any game content.
//-----------------------------------------------------------------------------

import :convars;
import :server;

import istd.bit;
import istd.crash;
import istd.fs;
import istd.io;
import istd.str;
import istd.uri;
import istd.uuid;
import istd;
import l14.client.command;
import l14.shared.args;
import l14.shared.b2;
import l14.shared.constants;
import l14.shared.http;
import l14.shared.pbar;
import l14.shared.sha256;
import l14.shared.zip;

namespace l14::ld
{

enum class DownloadingMethod
{
	Auto,
	Manifest,
	Zip,
};

[[nodiscard]] bool
DownloadingMethodFromArgs( const Args& args,
						   DownloadingMethod& method ) noexcept
{
	const auto val = args.Find( "method" );

	if ( val == nullptr )
		return true;

	if ( !val->IsNumber() ||
		 val->GetNumber() > static_cast<u8>( DownloadingMethod::Zip ) )
	{
		std::cerr << "Invalid downloading method" << val->GetNumber()
				  << std::endl;

		return false;
	}

	method = static_cast<DownloadingMethod>( val->GetNumber() );

	return true;
}

bool FetchServerInfo( const istd::Uri& httpServerUri,
					  sv::ServerInfo& serverInfo ) noexcept
{
	const auto response = sv::ServerInfoRequest( httpServerUri );
	const auto info = sv::ServerInfo::Parse( response );

	if ( !info.has_value() )
	{
		std::cerr << "Failed to get the server info" << std::endl;

		return false;
	}

	serverInfo = *info;

	if ( serverInfo.build.acz || serverInfo.build.downloadUrl.empty() )
	{
		serverInfo.build.downloadUrl =
			istd::Uri{ httpServerUri, "client.zip" }.ToString();

		serverInfo.build.manifestUrl =
			istd::Uri{ httpServerUri, "manifest.txt" }.ToString();

		serverInfo.build.manifestDownloadUrl =
			istd::Uri{ httpServerUri, "download" }.ToString();
	}

	return true;
}

void EnsureFolderExists( const std::filesystem::path& folder )
{
	if ( !istd::fs::Exists( folder ) )
		std::filesystem::create_directories( folder );
}

bool FileIsCached( const std::filesystem::path& cacheFolder,
				   const std::string_view hash )
{
	const auto cachedFilePath = cacheFolder / hash;

	if ( !istd::fs::Exists( cachedFilePath ) )
		return false;

	Sha256 sha256{};
	std::vector<char> hashBytes;

	try
	{
		hashBytes = istd::fs::ComputeHash( cachedFilePath, sha256 );
	}
	catch ( const std::exception& )
	{
		std::filesystem::remove( cachedFilePath );

		throw;
	}

	const auto cachedFileHash = Sha256::ToString( hashBytes );

	if ( !istd::str::CompareInsensitive( cachedFileHash, hash ) )
	{
		std::cerr << "The cached file has an invalid hash: " << cachedFileHash
				  << std::endl;

		std::filesystem::remove( cachedFilePath );

		return false;
	}

	return true;
}

struct DownloadedFile
{
	usize size{};
	std::string hash{};
};

[[nodiscard]] bool DownloadFile( const std::filesystem::path& filePath,
								 const std::string& downloadUri,
								 DownloadedFile& result )
{
	Sha256 sha256{};

	std::ofstream fileStream{ filePath, std::ios::binary | std::ios::trunc };
	fileStream.exceptions( std::ios::badbit );

	std::cerr << std::format( "Downloading '{}'", filePath.string() )
			  << std::endl;

	auto lastReport = std::chrono::steady_clock::now();

	const auto& http = HttpClient::Instance();
	const DownloadOptions options{
		.stream = fileStream,
		.progressCb = DefaultProgressCallback( lastReport ),
		.downloadCb =
			[&sha256]( const std::span<char> chunk )
		{
			sha256.Update( chunk );
		},
	};

	const auto response = http.Download( downloadUri, options );

	if ( !response.error.empty() )
	{
		std::cerr << std::format( "The request returned error: {}",
								  response.error )
				  << std::endl;

		return false;
	}

	const auto hashBytes = sha256.Final();

	result.size = fileStream.tellp();
	result.hash = Sha256::ToString( std::span{ hashBytes } );

	return true;
}

[[nodiscard]] bool ExtractZip( const std::filesystem::path& filePath,
							   const std::optional<usize> fileSize,
							   const std::filesystem::path& folder,
							   const bool overwrite )
{
	usize _fileSize;

	if ( fileSize.has_value() )
		_fileSize = *fileSize;
	else
	{
		std::ofstream stream{ filePath, std::ios::binary | std::ios::ate };
		stream.exceptions( std::ios::badbit );

		_fileSize = stream.tellp();
	}

	zip::Archive archive{};

	try
	{
		archive = zip::Archive::Open( filePath.string() );
	}
	catch ( const zip::Exception& err )
	{
		std::cerr << "Failed to open the downloaded archive: " << err.what()
				  << std::endl;

		return false;
	}

	const std::filesystem::path clientsFolder{
		g_clientsFolderCVar.Value().GetString() };

	if ( istd::fs::Exists( folder ) && !std::filesystem::is_empty( folder ) )
	{
		if ( !overwrite )
		{
			std::cerr << std::format( "The folder '{}' is not empty",
									  folder.string() )
					  << std::endl;

			return false;
		}

		std::filesystem::remove_all( folder );
	}

	std::filesystem::create_directories( folder );

	std::cerr << std::format( "Extracting to {}: ", folder.string() );

	ProgressBar pb{};

	try
	{

		archive.Extract( folder,
						 [&pb, _fileSize]( const usize extracted )
						 {
							 pb.Set( static_cast<float>( extracted ) /
									 static_cast<float>( _fileSize ) );
						 } );

		pb.End();
	}
	catch ( const zip::Exception& err )
	{
		pb.End();

		std::cerr << "Failed to extract the archive: " << err.what()
				  << std::endl;

		return false;
	}

	return true;
}

bool DownloadClientCommandCb( const CommandDef& def,
							  const std::span<const std::string_view> args )
{
	if ( args.size() < 2 )
	{
		std::cerr << std::format( "Command '{}' expects 2 arguments",
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

	std::filesystem::path folder = args[1];

	if ( folder.empty() || folder.has_filename() )
	{
		std::cerr << "Invalid folder path: " << folder.string() << std::endl;

		return false;
	}

	istd::Uri httpServerUri;

	if ( !sv::ToHttpUri( serverUri, httpServerUri ) )
	{
		std::cerr << "Invalid server uri: " << serverUri.ToString()
				  << std::endl;

		return false;
	}

	auto method = DownloadingMethod::Auto;
	bool overwrite = false;

	{
		Args _args{};
		_args.Parse( args.subspan( 2 ) );

		if ( !DownloadingMethodFromArgs( _args, method ) )
			return false;

		if ( const auto _overwrite = _args.Find( "overwrite" );
			 _overwrite != nullptr )
		{
			overwrite = _overwrite->GetBool();
		}
	}

	if ( istd::fs::Exists( folder ) && !std::filesystem::is_empty( folder ) )
	{
		if ( !overwrite )
		{
			std::cerr << "The specified folder is not empty" << std::endl;

			return false;
		}

		std::cerr << "The specified folder is not empty, deleting" << std::endl;

		std::filesystem::remove_all( folder );
	}

	std::filesystem::create_directories( folder );

	sv::ServerInfo serverInfo;

	if ( !FetchServerInfo( httpServerUri, serverInfo ) )
		return false;

	const auto& buildInfo = serverInfo.build;

	const auto hasManifestMethod = buildInfo.HasManifestDownloadingMethod();

	const auto hasZipMethod =
		buildInfo.HasZipDownloadingMethod() &&
		( !buildInfo.hash.empty() || !buildInfo.forkId.empty() );

	if ( method == DownloadingMethod::Auto )
	{
		if ( hasManifestMethod )
		{
			std::cerr << "Using the manifest downloading method" << std::endl;

			method = DownloadingMethod::Manifest;
		}
		else if ( hasZipMethod )
		{
			std::cerr << "Using the zip downloading method" << std::endl;

			method = DownloadingMethod::Zip;
		}
		else
		{
			std::cerr << "Neither a manifest nor an archive was provided for "
						 "download."
					  << std::endl;

			return false;
		}
	}

	switch ( method )
	{
	case DownloadingMethod::Auto:
		UNREACHABLE();
	case DownloadingMethod::Manifest:
	{
		if ( !hasManifestMethod )
		{
			std::cerr << "Manifest method not available" << std::endl;

			return false;
		}

		const auto manifestProto =
			sv::FetchManifestDownloadProtocol( buildInfo.manifestDownloadUrl );

		if ( !manifestProto.has_value() )
		{
			std::cerr << "Failed to fetch the protocol version" << std::endl;

			return false;
		}

		if ( manifestProto->maxVersion > k_manifestDownloadProtocolVersion )
		{
			std::cerr << std::format(
							 "Version of the protocol {} is not supported",
							 manifestProto->maxVersion )
					  << std::endl;

			return false;
		}

		std::string manifest;
		std::string manifestHash;

		// Fetch manifest
		{
			const RequestOptions options{
				.timeoutMs = static_cast<long>(
					g_httpTimeoutCVar.DefaultValue().GetNumber() ),
			};

			const auto& http = HttpClient::Instance();
			auto response = http.Get( buildInfo.manifestUrl, options );

			if ( !response.error.empty() )
			{
				std::cerr << "The response returned error: " << response.error
						  << std::endl;

				return false;
			}

			if ( response.code != 200 )
			{
				std::cerr
					<< "Failed to fetch the manifest, the response returned "
					<< response.code << std::endl;

				return false;
			}

			manifest =
				std::string{ response.content.data(), response.content.size() };

			if ( !manifest.starts_with( "Robust Content Manifest 1" ) )
			{
				std::cerr << "Invalid manifest format" << std::endl;

				return false;
			}

			{
				const auto _manifestHash =
					response.headers.find( k_manifestHashHeader );

				if ( _manifestHash == response.headers.end() )
					manifestHash = "";
				else
					manifestHash = _manifestHash->second;
			}

			if ( manifestHash.empty() )
			{
				std::cerr << "The response did not returned the manifests' hash"
						  << std::endl;

				if ( g_strictHashCVar.Value().GetBool() )
					return false;
			}
		}

		// Compute manifest hash
		{
			Blake2B b2{ 32 };

			b2.Update( std::span{ manifest.data(), manifest.size() } );

			const auto hashBytes = b2.Final();
			std::string hash = Blake2B::ToString( hashBytes );

			if ( !istd::str::CompareInsensitive( manifestHash, hash ) )
			{
				std::cerr << std::format( "Manifest hashes are not "
										  "equal\nExpected: {}, actual: {}",
										  manifestHash, hash )
						  << std::endl;

				if ( g_strictHashCVar.Value().GetBool() )
					return false;
			}
		}

		// Downloading
		{
			struct ManifestEntry final
			{
				std::string_view hash;
				std::string_view path;
			};

			std::vector<char> body{};
			std::vector<ManifestEntry> entries{};

			// Preparing request body
			{
				const auto lines = istd::str::Split( manifest, "\r\n" );

				if ( lines.size() - 1 >= std::numeric_limits<u32>::max() )
				{
					std::cerr
						<< "The manifest file is too long: " << lines.size() - 1
						<< std::endl;

					return false;
				}

				body.resize( ( lines.size() - 1 ) * sizeof( u32 ), '\0' );

				for ( size_t i = 1; i < lines.size(); i++ )
				{
					const auto pair = istd::str::Split( lines[i], " ", 1 );

					if ( pair.size() != 2 )
					{
						std::cerr << "Invalid manifest format" << std::endl;

						return false;
					}

					entries.emplace_back( istd::str::TrimSpaces( pair[0] ),
										  istd::str::TrimSpaces( pair[1] ) );

					auto index = static_cast<u32>( i - 1 );

					if constexpr ( std::endian::native != std::endian::little )
						index = istd::bit::ByteSwap( index );

					const auto indexBytes =
						std::bit_cast<std::array<char, sizeof( u32 )>>( index );

					std::memcpy( body.data() + ( index * 4 ), indexBytes.data(),
								 indexBytes.size() );
				}

				std::cerr << std::format( "Downloading {} files",
										  lines.size() - 1 )
						  << std::endl;
			}

			std::vector<char> data{};

			// TODO: Streaming

			// Fetching
			{
				auto lastReport = std::chrono::steady_clock::now();

				const RequestOptions options{
					.timeoutMs = static_cast<long>(
						g_httpTimeoutCVar.DefaultValue().GetNumber() ),
					.headers =
						{
							{ k_downloadProtocolHeader,
							  std::format(
								  "{}", k_manifestDownloadProtocolVersion ) },
							{ "content-type", "application/octet-stream" },
							{ "accept-encoding", "zstd" },
						},
					.body = body,
					.progressCb = DefaultProgressCallback( lastReport ),
				};

				const auto& http = HttpClient::Instance();
				auto response =
					http.Post( buildInfo.manifestDownloadUrl, options );

				if ( !response.error.empty() )
				{
					std::cerr
						<< "The request returned error: " << response.error
						<< std::endl;

					return false;
				}

				if ( response.code != 200 )
				{
					std::cerr << "The request returned: " << response.code
							  << std::endl;

					return false;
				}

				data = std::move( response.content );
			}

			if ( data.empty() )
			{
				std::cerr << "The request returned empty data" << std::endl;

				return false;
			}

			istd::BufferReader reader{ data };
			u32 header{};

			if ( !reader.ReadNumberLE( header ) )
			{
				std::cerr << "Unexpected EOF: expected header" << std::endl;

				return false;
			}

			bool preCompression{};

			if ( ( header & ( 1 << 0 ) ) == 1 )
			{
				// TODO: Did not found any server to test
				preCompression = true;

				std::cerr << "Compressed blobs are not supported" << std::endl;

				return false;
			}

			size_t fileIndex{};

			ProgressBar pb{};

			std::cerr << "Unpacking files: ";

			while ( fileIndex < entries.size() )
			{
				u32 contentSize{};

				if ( !reader.ReadNumberLE( contentSize ) )
				{
					std::cerr << "Unexpected EOF: expected file header"
							  << std::endl;

					return false;
				}

				const auto content = reader.Read( contentSize );

				if ( content.size() != contentSize )
				{
					std::cerr << "Unexpected EOF: expected content"
							  << std::endl;

					return false;
				}

				const auto& entry = entries[fileIndex];

				std::string hash;

				{
					Blake2B b2{ 32 };

					b2.Update( content );

					const auto hashBytes = b2.Final();

					hash = Blake2B::ToString( hashBytes );
				}

				if ( !istd::str::CompareInsensitive( hash, entry.hash ) )
				{
					std::cerr << std::format( "Hashes of file '{}' are not "
											  "equal\nExpected: {}, actual: {}",
											  entry.path, entry.hash, hash )
							  << std::endl;

					if ( g_strictHashCVar.Value().GetBool() )
						return false;
				}

				const std::filesystem::path filePath{ entry.path };
				const auto dstPath = folder / filePath;

				try
				{
					std::filesystem::create_directories(
						dstPath.parent_path() );
					std::ofstream fileStream{ dstPath, std::ios::binary };

					istd::fs::WriteAll( fileStream, content );
				}
				catch ( const std::ios::failure& err )
				{
					std::cerr
						<< std::format( "Failed to write to file '{}': {}",
										dstPath.string(), err.what() );

					return false;
				}

				fileIndex += 1;
				pb.Set( static_cast<float>( fileIndex ) /
						static_cast<float>( entries.size() ) );
			}
		}

		return true;
	}
	case DownloadingMethod::Zip:
	{
		if ( !hasZipMethod )
		{
			std::cerr << "Zip method not available" << std::endl;

			return false;
		}

		std::filesystem::path cacheFolder{ g_cacheDirCVar.Value().GetString() };

		EnsureFolderExists( cacheFolder );

		std::filesystem::path filePath;
		bool downloadFile;

		if ( buildInfo.hash.empty() )
		{
			std::cerr << "The server did not provide the hash of the client"
					  << std::endl;

			if ( g_strictHashCVar.Value().GetBool() )
				return false;

			downloadFile = true;
			filePath = cacheFolder / istd::UuidV4::Generate().ToString();
		}
		else
		{
			downloadFile = !FileIsCached( cacheFolder, buildInfo.hash );
			filePath = cacheFolder / buildInfo.hash;
		}

		usize fileSize;

		if ( downloadFile )
		{
			DownloadedFile result;

			if ( !DownloadFile( filePath, buildInfo.downloadUrl, result ) )
			{
				std::filesystem::remove( filePath );

				return false;
			}

			if ( !istd::str::CompareInsensitive( result.hash, buildInfo.hash ) )
			{
				std::cerr
					<< std::format(
						   "Hashes are not equal\nExpected: {}, actual: {}",
						   buildInfo.hash, result.hash )
					<< std::endl;

				if ( g_strictHashCVar.Value().GetBool() )
					return false;

				const auto newPath = filePath.parent_path() / result.hash;
				std::filesystem::rename( filePath, newPath );

				filePath = newPath;
			}

			fileSize = result.size;
		}
		else
		{
			std::cerr << "Client was found in the cache" << std::endl;
		}

		return ExtractZip( filePath, fileSize, folder, overwrite );
	}
	}

	UNREACHABLE();
}

} // namespace l14::ld
