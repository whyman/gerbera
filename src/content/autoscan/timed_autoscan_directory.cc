/*GRB*

Gerbera - https://gerbera.io/

    timed_autoscan_directory.cc - this file is part of Gerbera.

    Copyright (C) 2022 Gerbera Contributors

    Gerbera is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    Gerbera is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Gerbera.  If not, see <http://www.gnu.org/licenses/>.
*/

/// \file timed_autoscan_directory.cc

#include "timed_autoscan_directory.h"

#include <utility>

TimedAutoscanDirectory::TimedAutoscanDirectory(fs::path location, bool recursive, bool hidden, std::chrono::seconds interval, AutoscanSource source)
    : AutoscanDirectory(std::move(location),ScanMode::Timed,recursive,hidden,source), interval(interval)
{
}

std::shared_ptr<Timer::Parameter> TimedAutoscanDirectory::getTimerParameter() const
{
    return timer_parameter;
}
