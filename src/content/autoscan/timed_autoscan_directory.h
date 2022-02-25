/*GRB*

Gerbera - https://gerbera.io/

    timed_autoscan_directory.h - this file is part of Gerbera.

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

/// \file timed_autoscan_directory.h

#ifndef GERBERA_TIMED_AUTOSCAN_DIRECTORY_H
#define GERBERA_TIMED_AUTOSCAN_DIRECTORY_H

#include "autoscan_directory.h"

class TimedAutoscanDirectory : public AutoscanDirectory {
public:
    TimedAutoscanDirectory(fs::path location, bool recursive, bool hidden, std::chrono::seconds interval, AutoscanSource source);

    /// \brief Get the timer notify parameter associated with this directory.
    std::shared_ptr<Timer::Parameter> getTimerParameter() const;

    void setInterval(std::chrono::seconds interval) { this->interval = interval; }
    [[nodiscard]] std::chrono::seconds getInterval() const { return interval; }

private:
    std::chrono::seconds interval {};
    std::shared_ptr<Timer::Parameter> timer_parameter { std::make_shared<Timer::Parameter>(Timer::Parameter::IDAutoscan, AutoscanDirectory::INVALID_SCAN_ID) };
};

#endif // GERBERA_TIMED_AUTOSCAN_DIRECTORY_H
