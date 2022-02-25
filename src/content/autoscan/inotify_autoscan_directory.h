/*GRB*

Gerbera - https://gerbera.io/

    inotify_autoscan_directory.h - this file is part of Gerbera.

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

/// \file inotify_autoscan_directory.h

#ifndef GERBERA_INOTIFY_AUTOSCAN_DIRECTORY_H
#define GERBERA_INOTIFY_AUTOSCAN_DIRECTORY_H

#include "autoscan_directory.h"

class INotifyAutoscanDirectory : public AutoscanDirectory {
public:
    INotifyAutoscanDirectory(const fs::path& location, bool recursive, bool hidden, AutoscanSource source);
};

#endif // GERBERA_INOTIFY_AUTOSCAN_DIRECTORY_H
