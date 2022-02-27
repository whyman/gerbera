/*GRB*

Gerbera - https://gerbera.io/

    file_processor.h - this file is part of Gerbera.

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

/// \file FileProcessor.h

#ifndef GERBERA_FILE_PROCESSOR_H
#define GERBERA_FILE_PROCESSOR_H

#include "cds_objects.h"

class AutoScanSetting;

class FileProcessor {
public:
    // LEGACY API

    /* the functions below return true if the task has been enqueued */

    /// \brief Adds a file or directory to the database.
    /// \param dirEnt absolute path to the file
    /// \param recursive recursive add (process subdirecotories)
    /// \param async queue task or perform a blocking call
    /// \param hidden true allows to import hidden files, false ignores them
    /// \param rescanResource true allows to reload a directory containing a resource
    /// \param queue for immediate processing or in normal order
    /// \return object ID of the added file - only in blockign mode, when used in async mode this function will return INVALID_OBJECT_ID
    virtual int addFile(const fs::directory_entry& dirEnt, AutoScanSetting& asSetting,
        bool async, bool lowPriority, bool cancellable) = 0;

    /// \brief Adds a file or directory to the database.
    /// \param dirEnt absolute path to the file
    /// \param rootpath absolute path to the container root
    /// \param recursive recursive add (process subdirecotories)
    /// \param async queue task or perform a blocking call
    /// \param hidden true allows to import hidden files, false ignores them
    /// \param rescanResource true allows to reload a directory containing a resource
    /// \param queue for immediate processing or in normal order
    /// \return object ID of the added file - only in blockign mode, when used in async mode this function will return INVALID_OBJECT_ID
    virtual int addFile(const fs::directory_entry& dirEnt, const fs::path& rootpath, AutoScanSetting& asSetting,
        bool async, bool lowPriority, bool cancellable) = 0;
    virtual std::shared_ptr<CdsObject> createObjectFromFile(const fs::directory_entry& dirEnt, bool followSymlinks, bool allowFifo) = 0;
    virtual int ensurePathExistence(const fs::path& path) const = 0;
    virtual void rescanDirectory(const std::shared_ptr<AutoscanDirectory>& adir, int objectId, fs::path descPath = {}, bool cancellable = true) = 0;

    // NEW API
    virtual void onPathCreated(fs::path path) = 0;
    virtual void onPathUpdated(fs::path path) = 0;
    virtual void onPathRemoved(fs::path path) = 0;
};

#endif // GERBERA_FILE_PROCESSOR_H
