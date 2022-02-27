/*GRB*

Gerbera - https://gerbera.io/

    directory_monitor.h - this file is part of Gerbera.

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

/// \file directory_monitor.h

#ifndef GERBERA_DIRECTORY_MONITOR_H
#define GERBERA_DIRECTORY_MONITOR_H

#include "autoscan_directory.h"
#include "content/file_processor.h"

class DirectoryMonitor {
public:
    explicit DirectoryMonitor(FileProcessor& fileProcessor, bool followSymlinks) : fileProcessor(fileProcessor), followSymlinks(followSymlinks) {};

    /// \brief Start monitoring a directory
    virtual void monitor(AutoscanDirectory& dir) = 0;

    /// \brief Stop monitoring a directory
    virtual void unmonitor(AutoscanDirectory& dir) = 0;

protected:
    FileProcessor& fileProcessor;
    bool followSymlinks;
};

#endif // GERBERA_DIRECTORY_MONITOR_H
