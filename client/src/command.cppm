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
#include <functional>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <utility>

export module l14.client.command;

namespace l14
{

export using CommandCallback = std::function<bool(
	const class CommandDef& def, std::span<const std::string_view> args )>;

export class CommandDef final
{
  public:
	CommandDef() = delete;

	CommandDef( std::string name, CommandCallback callback,
				std::string desc = "", std::string help = "" )
		: m_name( std::move( name ) ), m_desc( std::move( desc ) ),
		  m_usage( std::move( help ) ), m_callback( std::move( callback ) )
	{
		mNext = sInstance;
		sInstance = this;
	}

	CommandDef( const CommandDef& ) = delete;
	CommandDef& operator=( const CommandDef& ) = delete;
	CommandDef( const CommandDef&& ) = delete;
	CommandDef& operator=( const CommandDef&& ) = delete;

	[[nodiscard]] std::string_view Name() const noexcept
	{
		return m_name;
	}

	[[nodiscard]] std::string_view Desc() const noexcept
	{
		return m_desc;
	}

	[[nodiscard]] std::string_view Usage() const noexcept
	{
		return m_usage;
	}

	[[nodiscard]] static CommandDef* List() noexcept
	{
		return sInstance;
	}

	[[nodiscard]] CommandDef* Next() const noexcept
	{
		return mNext;
	}

	[[nodiscard]] bool
	Invoke( const std::span<const std::string_view> args ) const noexcept
	{
		try
		{
			return m_callback( *this, args );
		}
		catch ( const std::exception& err )
		{
			std::cerr
				<< std::format(
					   "Fatal error occurred while executing '{}' command: {}",
					   m_name, err.what() )
				<< std::endl;

			return false;
		}
	}

  private:
	static CommandDef* sInstance;

	std::string m_name;
	std::string m_desc;
	std::string m_usage;
	CommandCallback m_callback;
	CommandDef* mNext{};
};

CommandDef* CommandDef::sInstance = nullptr;

} // namespace l14
