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

module;

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module l14.cmd.updater;

import istd.a4;
import istd.defer;
import istd.format.kv;
import istd.fs;
import istd.str;
import istd.uuid;
import istd;
import l14.shared.b2;
import l14.shared.constants;
import l14.shared.http;
import l14.shared.pbar;
import l14.shared.zip;

namespace l14::updater
{

struct GitHubReleaseAsset final
{
	std::string fileName{};
	std::string downloadUrl{};

	[[nodiscard]] static std::optional<std::vector<GitHubReleaseAsset>>
	Parse( const nlohmann::basic_json<>& data ) noexcept
	{
		if ( !data.is_array() )
			return std::nullopt;

		std::vector<GitHubReleaseAsset> assets{};

		for ( const auto& _asset : data )
		{
			if ( !_asset.is_object() )
				return std::nullopt;

			GitHubReleaseAsset asset{};

			// Parse name
			{
				const auto name = _asset.find( "name" );

				if ( name == _asset.end() || !name->is_string() )
					return std::nullopt;

				asset.fileName = *name;
			}

			// Parse download url
			{
				const auto downloadUrl = _asset.find( "browser_download_url" );

				if ( downloadUrl == _asset.end() || !downloadUrl->is_string() )
					return std::nullopt;

				asset.downloadUrl = *downloadUrl;
			}

			assets.push_back( asset );
		}

		return assets;
	}
};

struct GitHubRelease final
{
	[[nodiscard]] static std::optional<GitHubRelease>
	Parse( const nlohmann::basic_json<>& data ) noexcept
	{
		if ( !data.is_object() )
			return std::nullopt;

		GitHubRelease release{};

		// Parse version
		{
			const auto tagName = data.find( "tag_name" );

			if ( tagName == data.end() || !tagName->is_string() )
				return std::nullopt;

			const std::string& _tagName = *tagName;

			if ( !_tagName.starts_with( 'v' ) )
				return std::nullopt;

			const auto version =
				istd::str::FromChars<usize>( _tagName.substr( 1 ) );

			if ( !version.has_value() )
				return std::nullopt;

			release.version = *version;
		}

		// Parse assets
		{
			const auto assets = data.find( "assets" );

			if ( assets == data.end() )
				return std::nullopt;

			auto _assets = GitHubReleaseAsset::Parse( *assets );

			if ( !_assets.has_value() )
				return std::nullopt;

			release.assets = std::move( *_assets );
		}

		return release;
	}

	usize version{};
	std::vector<GitHubReleaseAsset> assets{};
};

std::optional<GitHubRelease> FetchLastRelease() noexcept
{
	const auto& http = HttpClient::Instance();

	const RequestOptions options{
		.timeoutMs = 8000,
		.headers =
			{
				{ "user-agent", "l14" },
				{ "accept", "application/vnd.github+json" },
				{ "x-gitHub-api-version", "2022-11-28" },
			},
	};
	const auto response = http.Get( k_githubReleaseUrl, options );

	if ( response.code != 200 )
	{
		std::cerr << "Failed to check the latest update: code " << response.code
				  << std::endl;

		return std::nullopt;
	}

	nlohmann::basic_json<> json{};

	try
	{
		json = nlohmann::json::parse( response.content );
	}
	catch ( const nlohmann::json::parse_error& err )
	{
		std::cerr << "Failed to parse the update info: " << err.what()
				  << std::endl;

		return std::nullopt;
	}

	return GitHubRelease::Parse( json );
}

struct FileInfo final
{
	[[nodiscard]] static FileInfo
	Deserialize( const istd::a4::Object& object ) noexcept
	{
		FileInfo fileInfo{};

		{
			const auto hash = object.value.find( "hash" );

			if ( hash != object.value.end() &&
				 std::holds_alternative<istd::a4::String>( hash->second ) )
			{
				fileInfo.hash =
					std::get<istd::a4::String>( hash->second ).value;
			}
		}

		return fileInfo;
	}

	std::string hash{};
};

struct Manifest final
{
	[[nodiscard]] static Manifest
	Deserialize( const istd::a4::Object& object ) noexcept
	{
		Manifest manifest{};

		{
			const auto version = object.value.find( "version" );

			if ( version != object.value.end() &&
				 std::holds_alternative<istd::a4::Number>( version->second ) )
			{
				const auto _version =
					std::get<istd::a4::Number>( version->second ).value;

				manifest.version = static_cast<usize>( _version );
			}
		}

		{
			const auto files = object.value.find( "files" );

			if ( files != object.value.end() &&
				 std::holds_alternative<istd::a4::Object>( files->second ) )
			{
				const auto _files = std::get<istd::a4::Object>( files->second );

				for ( const auto& [key, value] : _files.value )
				{
					if ( !std::holds_alternative<istd::a4::Object>( value ) )
						continue;

					auto fileInfo = FileInfo::Deserialize(
						std::get<istd::a4::Object>( value ) );

					manifest.files.insert_or_assign( key,
													 std::move( fileInfo ) );
				}
			}
		}

		return manifest;
	}

	usize version{};
	std::unordered_map<std::string, FileInfo> files{};
};

Manifest ParseManifest( const std::filesystem::path& manifestPath )
{
	std::ifstream stream{ manifestPath, std::ios::binary };
	stream.exceptions( std::ios::badbit );

	const auto content = istd::fs::ReadAll( stream );

	const auto [data, error] = istd::format::kv::Parse(
		std::string_view{ content.data(), content.size() } );

	if ( error.has_value() || !data.IsObject() )
	{
		std::cerr << "Failed to parse the update manifest: " << error->message
				  << std::endl;

		return { /* ok */ false };
	}

	const auto serialized = istd::format::kv::Serialize( data.GetObject() );

	return Manifest::Deserialize( serialized );
}

export struct UpdateResult final
{
	bool ok{};
	bool needsRestart{};
};

export [[nodiscard]] UpdateResult
RunAutoUpdate( const std::filesystem::path& exePath )
{
	GitHubRelease lastRelease;

	{
		auto _lastRelease = FetchLastRelease();

		if ( !_lastRelease.has_value() )
		{
			std::cerr << "No release was found" << std::endl;

			return UpdateResult{ /* ok */ false };
		}

		lastRelease = std::move( *_lastRelease );
	}

	if ( k_version >= lastRelease.version )
		return UpdateResult{ /* ok */ true };

	std::optional<GitHubReleaseAsset> asset{};

	for ( auto& _asset : lastRelease.assets )
	{
		if ( _asset.fileName == k_githubReleaseAssetName )
		{
			asset = std::move( _asset );

			break;
		}
	}

	if ( !asset.has_value() )
		return UpdateResult{ /* ok */ false };

	const auto assetRandomName = istd::UuidV4::Generate().ToString();
	const auto assetDownloadPath = std::filesystem::temp_directory_path() /
								   std::format( "{}.zip", assetRandomName );

	const auto _deferAssetRemove = istd::Defer(
		[&assetDownloadPath]
		{
			try
			{
				std::filesystem::remove( assetDownloadPath );
			}
			catch ( const std::exception& )
			{
			}
		} );

	// Download
	{
		std::cerr << "Downloading update" << std::endl;

		auto lastReport = std::chrono::steady_clock::now();

		std::ofstream stream{ assetDownloadPath, std::ios::binary };
		const DownloadOptions options{
			.stream = stream,
			.progressCb = DefaultProgressCallback( lastReport ),
			.downloadCb = std::nullopt,
		};
		const auto& http = HttpClient::Instance();
		const auto ret = http.Download( asset->downloadUrl, options );

		if ( !ret.error.empty() )
		{
			std::cerr << "Failed to download the update: " << ret.error
					  << std::endl;

			return UpdateResult{ /* ok */ false };
		}
	}

	const auto assetFolder =
		std::filesystem::temp_directory_path() / assetRandomName;

	const auto _deferAssetFolderRemove = istd::Defer(
		[&assetFolder]
		{
			try
			{
				std::filesystem::remove_all( assetFolder );
			}
			catch ( const std::exception& )
			{
			}
		} );

	std::filesystem::create_directory( assetFolder );

	try
	{
		const auto assetArchive =
			zip::Archive::Open( assetDownloadPath.string() );

		assetArchive.Extract( assetFolder, std::nullopt );
	}
	catch ( const zip::Exception& err )
	{
		std::cerr << "Failed to unpack the update: " << err.what() << std::endl;

		return UpdateResult{ /* ok */ false };
	}

	Manifest manifest{};

	try
	{
		manifest = ParseManifest( assetFolder / k_updateManifestName );
	}
	catch ( const std::exception& err )
	{
		std::cerr << "Failed to read the update manifest: " << err.what()
				  << std::endl;

		return UpdateResult{ /* ok */ false };
	}

	bool needsRestart = false;
	const auto exeDir = exePath.parent_path();

	// Compare hashes
	{
		for ( const auto& [path, info] : manifest.files )
		{
			const auto _path = istd::fs::GetAbsolutePath( path, assetFolder );

			std::vector<char> hashBytes;

			{
				l14::Blake2B b2{};

				hashBytes = istd::fs::ComputeHash( _path, b2 );
			}

			const auto hash = l14::Blake2B::ToString( hashBytes );

			if ( !istd::str::CompareInsensitive( hash, info.hash ) )
			{
				std::cerr << std::format( "Hashes are not equal for "
										  "'{}'\nExpected: {}, actual: {}",
										  path, info.hash, hash )
						  << std::endl;
			}
		}
	}

	// Apply update
	for ( const auto& [path, info] : manifest.files )
	{
		const std::filesystem::path _path{ path };

		if ( _path.empty() || _path.is_absolute() || !_path.has_filename() )
		{
			std::cerr << "Invalid file path: " << path << std::endl;

			return { /* ok */ false };
		}

		const auto fromPath = istd::fs::GetAbsolutePath( path, assetFolder );
		const auto toPath = istd::fs::GetAbsolutePath( path, exeDir );

		if ( toPath == exePath )
		{
			needsRestart = true;

			auto newExePath = exePath;
			newExePath.replace_filename(
				std::format( "{}{}.old", exePath.stem().string(),
							 exePath.extension().string() ) );

			std::filesystem::remove( newExePath );
			std::filesystem::rename( exePath, newExePath );
		}
		else
		{
			std::filesystem::create_directories( toPath.parent_path() );
		}

		std::filesystem::copy_file(
			fromPath, toPath,
			std::filesystem::copy_options::overwrite_existing );
	}

	return {
		.ok = true,
		.needsRestart = needsRestart,
	};
}

} // namespace l14::updater
