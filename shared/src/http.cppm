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
#include <array>
#include <chrono>
#include <cstring>
#include <curl/curl.h>
#include <curl/easy.h>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

export module l14.shared.http;

import istd.crash;
import istd.str;
import istd;
import l14.shared.constants;

namespace l14
{

class CurlList final
{
  public:
	CurlList() = default;

	CurlList( const CurlList& ) = delete;
	CurlList* operator=( const CurlList& ) = delete;

	CurlList( CurlList&& other ) noexcept
		: m_list( std::exchange( other.m_list, nullptr ) )
	{
	}

	CurlList& operator=( CurlList&& other ) noexcept
	{
		std::swap( m_list, other.m_list );

		return *this;
	}

	bool operator==( const std::nullptr_t& ) const
	{
		return m_list == nullptr;
	}

	const curl_slist* operator*() const noexcept
	{
		return m_list;
	}

	void Push( const std::string& text ) noexcept
	{
		const auto temp = curl_slist_append( m_list, text.c_str() );

		ASSERT( temp != nullptr );

		m_list = temp;
	}

	~CurlList()
	{
		if ( m_list == nullptr )
			return;

		curl_slist_free_all( std::exchange( m_list, nullptr ) );
	}

  private:
	curl_slist* m_list{};
};

export class HttpClient;

std::unique_ptr<HttpClient> g_client{};

struct EasyWriteData
{
	std::vector<char> contentBuffer{};
	std::vector<char> headersBuffer{};
};

size_t EasyWriteCallback( const char* ptr, const size_t size,
						  const size_t nmemb, void* userdata ) noexcept
{
	const auto realSize = size * nmemb;
	const auto data = static_cast<EasyWriteData*>( userdata );
	const auto oldSize = data->contentBuffer.size();

	data->contentBuffer.resize( data->contentBuffer.size() + realSize, '\0' );
	std::memcpy( data->contentBuffer.data() + oldSize, ptr, realSize );

	return realSize;
}

size_t EasyHeaderCallback( const char* ptr, const size_t size,
						   const size_t nitems, void* userdata ) noexcept
{
	const auto realSize = size * nitems;
	const auto data = static_cast<EasyWriteData*>( userdata );
	const auto oldSize = data->headersBuffer.size();

	data->headersBuffer.resize( data->headersBuffer.size() + realSize, '\0' );
	std::memcpy( data->headersBuffer.data() + oldSize, ptr, realSize );

	return realSize;
}

export using ProgressCallback =
	std::function<void( size_t downloaded, size_t total )>;

export template <typename Clock>
[[nodiscard]] ProgressCallback
DefaultProgressCallback( std::chrono::time_point<Clock>& lastReport ) noexcept
{
	return [&lastReport]( const size_t downloaded, const size_t total )
	{
		if ( downloaded == 0 && total == 0 )
			return;

		const auto now = std::chrono::steady_clock::now();

		if ( now - lastReport < k_downloadingProgressFrequency )
			return;

		lastReport = now;

		if ( total == 0 )
		{
			std::cerr << std::format( "Progress: {} / ?", downloaded )
					  << std::endl;
		}
		else
		{
			const auto percent = static_cast<float>( downloaded ) /
								 static_cast<float>( total ) * 100.0;

			std::cerr << std::format( "Progress {:.2f} ({} / {})", percent,
									  downloaded, total )
					  << std::endl;
		}
	};
}

struct RequestProgressData
{
	std::optional<ProgressCallback> progressCb;
};

int RequestProgressCallback( void* clientp, const curl_off_t dltotal,
							 const curl_off_t dlnow, const curl_off_t,
							 const curl_off_t ) noexcept
{
	if ( dltotal == 0 && dlnow == 0 )
		return 0;

	const auto data = static_cast<RequestProgressData*>( clientp );

	if ( data->progressCb.has_value() )
		data->progressCb.value()( dlnow, dltotal );

	return 0;
}

export struct RequestOptions
{
	long timeoutMs{};
	std::unordered_map<std::string, std::string> headers{};
	std::optional<std::vector<char>> body{};
	std::optional<ProgressCallback> progressCb;
};

export struct RequestResponse
{
	std::vector<char> content{};
	std::unordered_map<std::string, std::string> headers{};
	u32 code;
	std::string error{};
};

export struct DownloadResult
{
	std::string error{};
};

export using DownloadCallback = std::function<void( std::span<char> chunk )>;

export struct DownloadOptions
{
	std::ofstream& stream;
	std::optional<ProgressCallback> progressCb;
	std::optional<DownloadCallback> downloadCb;
};

struct DownloadWriteData
{
	std::ofstream& stream;
	std::string error{};
	std::optional<DownloadCallback> downloadCb;
};

size_t DownloadWriteCallback( const char* ptr, const size_t size,
							  const size_t nmemb, void* userdata ) noexcept
{
	const auto realSize = size * nmemb;
	const auto data = static_cast<DownloadWriteData*>( userdata );

	if ( data->downloadCb.has_value() )
	{
		std::vector<char> buffer{};
		buffer.resize( realSize, '\0' );

		std::memcpy( buffer.data(), ptr, realSize );

		( *data->downloadCb )( std::span{ buffer } );
	}

	try
	{
		for ( size_t i = 0; i < realSize; i++ )
			data->stream << ptr[i];
	}
	catch ( const std::ios::failure& err )
	{
		data->error = err.what();

		return CURL_WRITEFUNC_ERROR;
	}

	return realSize;
}

export class HttpClient final
{
  public:
	HttpClient( const HttpClient& ) = delete;
	HttpClient& operator=( const HttpClient& ) = delete;

	[[nodiscard]] static HttpClient& Instance() noexcept
	{
		if ( g_client == nullptr )
		{
			curl_global_init( CURL_GLOBAL_DEFAULT );

			const auto ptr = new HttpClient{};

			g_client = std::unique_ptr<HttpClient>( ptr );
		}

		return *g_client;
	}

	// NOTE: Non-static methods ensure that the client exists.

	[[nodiscard]] RequestResponse
	Get( const std::string& uri, const RequestOptions& options ) const noexcept
	{
		return Request( uri, "GET", options );
	}

	[[nodiscard]] RequestResponse
	Post( const std::string& uri, const RequestOptions& options ) const noexcept
	{
		return Request( uri, "POST", options );
	}

	[[nodiscard]] RequestResponse
	Options( const std::string& uri,
			 const RequestOptions& options ) const noexcept
	{
		return Request( uri, "OPTIONS", options );
	}

	[[nodiscard]] DownloadResult
	Download( const std::string& uri, const DownloadOptions& options ) const
	{
		DownloadWriteData userData{
			.stream = options.stream,
			.downloadCb = options.downloadCb,
		};
		RequestProgressData progressData{
			.progressCb = options.progressCb,
		};

		userData.stream.exceptions( std::ios::badbit );

		std::array<char, CURL_ERROR_SIZE> errorBuffer{};

		const auto handle = curl_easy_init();

		curl_easy_setopt( handle, CURLOPT_URL, uri.c_str() );
		curl_easy_setopt( handle, CURLOPT_WRITEFUNCTION,
						  DownloadWriteCallback );
		curl_easy_setopt( handle, CURLOPT_WRITEDATA, &userData );
		curl_easy_setopt( handle, CURLOPT_ERRORBUFFER, errorBuffer.data() );
		curl_easy_setopt( handle, CURLOPT_NOPROGRESS, 0 );
		curl_easy_setopt( handle, CURLOPT_XFERINFOFUNCTION,
						  RequestProgressCallback );
		curl_easy_setopt( handle, CURLOPT_XFERINFODATA, &progressData );
		curl_easy_setopt( handle, CURLOPT_FOLLOWLOCATION, 1 );

		const auto code = curl_easy_perform( handle );

		curl_easy_cleanup( handle );

		DownloadResult result{};

		if ( !userData.error.empty() )
			result.error = std::move( userData.error );
		else if ( code != CURLE_OK )
		{
			result.error = std::format( "{}: {}", curl_easy_strerror( code ),
										errorBuffer.data() );
		}

		return result;
	}

	~HttpClient()
	{
		curl_global_cleanup();
	}

  private:
	HttpClient() = default;

	static RequestResponse Request( const std::string& uri, const char* method,
									const RequestOptions& options ) noexcept
	{
		EasyWriteData userData{};
		userData.contentBuffer.reserve( 1024 );

		RequestProgressData progressData{
			.progressCb = options.progressCb,
		};

		std::array<char, CURL_ERROR_SIZE> errorBuffer{};

		const auto handle = curl_easy_init();

		curl_easy_setopt( handle, CURLOPT_URL, uri.c_str() );
		curl_easy_setopt( handle, CURLOPT_CUSTOMREQUEST, method );
		curl_easy_setopt( handle, CURLOPT_HEADERFUNCTION, EasyHeaderCallback );
		curl_easy_setopt( handle, CURLOPT_HEADERDATA, &userData );
		curl_easy_setopt( handle, CURLOPT_WRITEFUNCTION, EasyWriteCallback );
		curl_easy_setopt( handle, CURLOPT_WRITEDATA, &userData );
		curl_easy_setopt( handle, CURLOPT_ERRORBUFFER, errorBuffer.data() );
		curl_easy_setopt( handle, CURLOPT_FOLLOWLOCATION, 1 );

		if ( options.progressCb.has_value() )
		{
			curl_easy_setopt( handle, CURLOPT_NOPROGRESS, 0 );
			curl_easy_setopt( handle, CURLOPT_XFERINFOFUNCTION,
							  RequestProgressCallback );
			curl_easy_setopt( handle, CURLOPT_XFERINFODATA, &progressData );
		}
		else
		{
			curl_easy_setopt( handle, CURLOPT_TIMEOUT_MS,
							  static_cast<long>( options.timeoutMs ) );
		}

		if ( options.body.has_value() )
		{
			curl_easy_setopt( handle, CURLOPT_POSTFIELDS,
							  options.body->data() );
			curl_easy_setopt( handle, CURLOPT_POSTFIELDSIZE,
							  options.body->size() );
		}

		std::vector<std::string> headers{};
		headers.reserve( options.headers.size() );

		CurlList headerList{};

		for ( const auto& [key, value] : options.headers )
		{
			if ( istd::str::CompareInsensitive( key, "user-agent" ) )
			{
				curl_easy_setopt( handle, CURLOPT_USERAGENT, value.c_str() );

				continue;
			}

			if ( istd::str::CompareInsensitive( key, "accept-encoding" ) )
			{
				curl_easy_setopt( handle, CURLOPT_ACCEPT_ENCODING,
								  value.c_str() );

				continue;
			}

			headers.emplace_back( std::format( "{}: {}", key, value ) );
			headerList.Push( headers.back() );
		}

		curl_easy_setopt( handle, CURLOPT_HTTPHEADER, *headerList );

		const auto code = curl_easy_perform( handle );

		curl_easy_cleanup( handle );

		RequestResponse result{};

		if ( code != CURLE_OK )
		{
			result.error = std::format( "{}: {}", curl_easy_strerror( code ),
										errorBuffer.data() );
		}
		else
		{
			result.content = std::move( userData.contentBuffer );

			ParseHeaders( userData, result );
		}

		return result;
	}

	static void ParseHeaders( const EasyWriteData& easyData,
							  RequestResponse& request ) noexcept
	{
		if ( !request.error.empty() )
			return;

		if ( easyData.headersBuffer.empty() )
			return;

		const auto headers =
			istd::str::Split( std::string_view{ easyData.headersBuffer.data(),
												easyData.headersBuffer.size() },
							  "\r\n" );

		if ( headers.empty() )
		{
			request.error = "Invalid response headers";

			return;
		}

		{
			const auto values = istd::str::Split( headers[0], " ", 2 );

			if ( values.size() < 2 )
				return;

			const auto code = istd::str::FromChars<u32>( values[1] );

			if ( !code.has_value() )
			{
				request.error = "Invalid response status";

				return;
			}

			request.code = *code;
		}

		for ( size_t i = 1; i < headers.size(); i++ )
		{
			const auto pair = istd::str::Split( headers[i], ":", 1 );

			if ( pair.size() != 2 )
				continue;

			const auto key =
				istd::str::ToLower( istd::str::TrimSpaces( pair[0] ) );

			const auto value = istd::str::TrimSpaces( pair[1] );

			if ( key.empty() || value.empty() )
				continue;

			request.headers.insert_or_assign( std::string{ key },
											  std::string{ value } );
		}
	}
};

} // namespace l14
