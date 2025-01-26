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

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module istd.a4;

namespace istd::a4
{

export struct String final
{
	std::string value;
};

export struct Number final
{
	double value;
};

export struct Bool final
{
	bool value;
};

export struct Array final
{
	std::vector<std::variant<String, Number, Bool, Array, struct Object>> value;
};

export struct Object final
{
	std::unordered_map<std::string,
					   std::variant<String, Number, Bool, Array, Object>>
		value;
};

} // namespace istd::a4
