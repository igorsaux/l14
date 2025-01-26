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

#include "istd/platform.hpp"

export module l14.client;

import istd.crash;
import l14.client.cmd;
import l14.client.command;
import l14.shared.client;
import l14.client.globals;

namespace l14
{

class L14Client final : public IL14Client
{
  public:
	[[nodiscard]] IL14Cmd* CreateCmd() noexcept override
	{
		if ( g_cmd != nullptr )
			return g_cmd;

		g_cmd = new L14Cmd{};

		return g_cmd;
	}

	void DestroyCmd() noexcept override
	{
		if ( g_cmd == nullptr )
			return;

		delete g_cmd;
		g_cmd = nullptr;
	}
};

} // namespace l14

l14::L14Client g_client{};

L14_DLL_EXPORT void* getApp()
{
	return &g_client;
}
