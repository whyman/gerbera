/*MT*

    MediaTomb - http://www.mediatomb.cc/

    autoscan.cc - this file is part of MediaTomb.

    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.cc>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>

    Copyright (C) 2006-2010 Gena Batyan <bgeradz@mediatomb.cc>,
                            Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>,
                            Leonhard Wimmer <leo@mediatomb.cc>

    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    $Id$
*/

/// \file autoscan.cc

#include "autoscan.h" // API

#include "content_manager.h"
#include "database/database.h"

#include <fmt/chrono.h>

AutoscanDirectory::AutoscanDirectory()
    : timer_parameter(std::make_shared<Timer::Parameter>(Timer::Parameter::IDAutoscan, INVALID_SCAN_ID))
{
}

AutoscanDirectory::AutoscanDirectory(fs::path location, ScanMode mode, bool recursive, bool persistent,
    int id, unsigned int interval, bool hidden)
    : location(std::move(location))
    , mode(mode)
    , recursive(recursive)
    , hidden(hidden)
    , persistent_flag(persistent)
    , interval(interval)
    , scanID(id)
    , timer_parameter(std::make_shared<Timer::Parameter>(Timer::Parameter::IDAutoscan, INVALID_SCAN_ID))
{
}

void AutoscanDirectory::setPathLastModified(const std::string& loc, const std::filesystem::file_time_type& lmt)
{
    if (!loc.empty()) {
        pathLastModified[loc] = lmt;
    }
    if (lmt > lastModfied) {
        lastModfied = lmt;
    }
}

bool AutoscanDirectory::lastModHasChanged() const
{
    return lastModifiedUpdated;
}

fs::file_time_type AutoscanDirectory::getPathLastModified(const std::string& loc) const
{
    try {
        return pathLastModified.at(loc);
    } catch (std::out_of_range& e) {
        return fs::file_time_type{std::chrono::seconds::zero()};
    }
}

fs::file_time_type AutoscanDirectory::getScanLastModified() const {
    return lastModfied;
}

void AutoscanDirectory::setLocation(fs::path location)
{
    this->location = std::move(location);
}

void AutoscanDirectory::setScanID(int id)
{
    scanID = id;
    timer_parameter->setID(id);
}

std::string AutoscanDirectory::mapScanMode(ScanMode scanMode)
{
    std::string scanmode_str;
    switch (scanMode) {
    case ScanMode::Timed:
        scanmode_str = "timed";
        break;
    case ScanMode::INotify:
        scanmode_str = "inotify";
        break;
    default:
        throw_std_runtime_error("Illegal scanMode ({}) given to mapScanMode()", scanMode);
    }
    return scanmode_str;
}

ScanMode AutoscanDirectory::remapScanmode(const std::string& scanmode)
{
    if (scanmode == "timed")
        return ScanMode::Timed;
    if (scanmode == "inotify")
        return ScanMode::INotify;

    throw_std_runtime_error("Illegal scanmode ({}) given to remapScanmode()", scanmode.c_str());
}

void AutoscanDirectory::copyTo(const std::shared_ptr<AutoscanDirectory>& copy) const
{
    copy->location = location;
    copy->mode = mode;
    copy->recursive = recursive;
    copy->hidden = hidden;
    copy->persistent_flag = persistent_flag;
    copy->interval = interval;
    copy->taskCount = taskCount;
    copy->scanID = scanID;
    copy->objectID = objectID;
    copy->databaseID = databaseID;
    copy->lastModfied = lastModfied;
    copy->timer_parameter = timer_parameter;
    copy->lastModifiedUpdated = lastModifiedUpdated;
}

std::shared_ptr<Timer::Parameter> AutoscanDirectory::getTimerParameter()
{
    return timer_parameter;
}
