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

#include "autoscan_directory.h" // API

#include "content/content_manager.h"
#include "database/database.h"

AutoscanDirectory::AutoscanDirectory(fs::path location, ScanMode mode, bool recursive, bool hidden, AutoscanSource source)
    : location(std::move(location))
    , mode(mode)
    , recursive(recursive)
    , hidden(hidden)
    , source(source)
{
}

void AutoscanDirectory::setCurrentLMT(const fs::path& loc, std::chrono::seconds lmt)
{
    bool firstScan = false;
    bool activeScan = false;
    if (!loc.empty()) {
        auto lmDir = lastModified.find(loc);
        if (lmDir == lastModified.end() || lastModified.at(loc) > std::chrono::seconds::zero()) {
            firstScan = true;
        }
        if (lmDir == lastModified.end() || lastModified.at(loc) == std::chrono::seconds::zero()) {
            activeScan = true;
        }
        lastModified[loc] = lmt;
    }
    if (lmt == std::chrono::seconds::zero()) {
        if (firstScan) {
            activeScanCount++;
        }
    } else {
        if (activeScan && activeScanCount > 0) {
            activeScanCount--;
        }
        if (lmt > last_mod_current_scan) {
            last_mod_current_scan = lmt;
        }
    }
}

bool AutoscanDirectory::updateLMT()
{
    bool result = taskCount <= 0 && activeScanCount == 0;
    if (result) {
        result = last_mod_previous_scan < last_mod_current_scan;
        last_mod_previous_scan = last_mod_current_scan;
        log_debug("set autoscan lmt location: {}; last_modified: {}", location.c_str(), last_mod_current_scan.count());
    }
    return result;
}

std::chrono::seconds AutoscanDirectory::getPreviousLMT(const fs::path& loc, const std::shared_ptr<CdsContainer>& parent) const
{
    auto lmDir = lastModified.find(loc);
    if (!loc.empty() && lmDir != lastModified.end() && lastModified.at(loc) > std::chrono::seconds::zero()) {
        return lastModified.at(loc);
    }
    if (parent && parent->getMTime() > std::chrono::seconds::zero()) {
        return parent->getMTime();
    }
    return last_mod_previous_scan;
}

std::chrono::seconds AutoscanDirectory::getPreviousLMT() const
{
    return last_mod_previous_scan;
}

void AutoscanDirectory::setLocation(const fs::path& location)
{
    this->location = location;
}

std::string_view AutoscanDirectory::mapScanMode(ScanMode scanmode)
{
    switch (scanmode) {
    case ScanMode::Timed:
        return "timed";
    case ScanMode::INotify:
        return "inotify";
    }
    throw_std_runtime_error("Illegal scanmode ({}) given to mapScanMode()", scanmode);
}

ScanMode AutoscanDirectory::remapScanMode(const std::string& scanmode)
{
    if (scanmode == "timed")
        return ScanMode::Timed;
    if (scanmode == "inotify")
        return ScanMode::INotify;

    throw_std_runtime_error("Illegal scanmode ({}) given to remapScanMode()", scanmode);
}
std::string_view AutoscanDirectory::mapAutoscanSource(AutoscanSource source)
{
    switch (source) {
    case AutoscanSource::Web:
        return "Web";
    case AutoscanSource::ConfigFile:
        return "ConfigFile";
    }
    throw_std_runtime_error("Illegal source ({}) given to mapAutoscanSource()", source);
}
