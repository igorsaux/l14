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

#include <chrono>

export module l14.shared.constants;

namespace l14
{

export constexpr auto k_version = 2;
export constexpr auto k_manifestDownloadProtocolVersion = 1;
export constexpr auto k_configFileName = "config.cfg";
export constexpr auto k_httpDefaultServerPort = 1212;
export constexpr auto k_httpsDefaultServerPort = 443;
export constexpr auto k_downloadingProgressFrequency =
	std::chrono::seconds( 5 );

#ifdef L14_IS_WINDOWS
export constexpr auto k_clientDll = "l14.dll";
#elif L14_IS_LINUX
export constexpr auto k_clientDll = "libl14.so";
#else
	#error The OS is not supported
#endif

export constexpr auto k_downloadMinProtocolHeader =
	"x-robust-download-min-protocol";
export constexpr auto k_downloadMaxProtocolHeader =
	"x-robust-download-max-protocol";
export constexpr auto k_manifestHashHeader = "x-manifest-hash";
export constexpr auto k_downloadProtocolHeader = "x-robust-download-protocol";

export constexpr auto k_githubReleaseUrl =
	"https://api.github.com/repos/igorsaux/l14/releases/latest";

export constexpr auto k_updateManifestName = "manifest.txt";

#ifdef L14_IS_WINDOWS
export constexpr auto k_githubReleaseAssetName = "x86_64-windows.zip";
#elif L14_IS_LINUX
export constexpr auto k_githubReleaseAssetName = "x86_64-linux.zip";
#else
	#error The OS is not supported
#endif

} // namespace l14
