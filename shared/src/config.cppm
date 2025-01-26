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

#include <variant>

export module l14.shared.config;

// NOTE: Stub

import istd.a4;

namespace l14
{

export struct Config final
{
	[[nodiscard]] static const Config& Default() noexcept
	{
		return s_default;
	}

	[[nodiscard]] static Config
	Deserialize( const istd::a4::Object& object ) noexcept
	{
		using namespace istd::a4;

		return Config{};
	}

	[[nodiscard]] istd::a4::Object Serialize() const noexcept
	{
		using namespace istd::a4;

		return Object{};
	}

  private:
	const static Config s_default;
};

const Config Config::s_default = Config{};

} // namespace l14
