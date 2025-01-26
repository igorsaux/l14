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

export module l14.shared.client;

import l14.shared.app;
import l14.shared.config;

namespace l14
{

export class IL14Cmd
{
  public:
	virtual int Run( int argc, char* argv[] ) noexcept = 0;

	virtual ~IL14Cmd() = default;
};

export class IL14Client : public IApp
{
  public:
	[[nodiscard]] virtual IL14Cmd* CreateCmd() noexcept = 0;

	virtual void DestroyCmd() noexcept = 0;
};

} // namespace l14
