/*GRB*

    Gerbera - https://gerbera.io/

    autoscan_list.h - this file is part of Gerbera.

    Copyright (C) 2021-2022 Gerbera Contributors

    Gerbera is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    Gerbera is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Gerbera.  If not, see <http://www.gnu.org/licenses/>.

    $Id$
*/

/// \file autoscan_list.h

#ifndef __AUTOSCAN_LIST_H__
#define __AUTOSCAN_LIST_H__

#include "util/timer.h"
#include "autoscan_inotify.h"

// forward declaration
class Database;
class AutoscanDirectory;
enum class ScanMode;

class AutoscanManager final {
public:
    explicit AutoscanManager(std::shared_ptr<Database> database, std::shared_ptr<Config> config);

    /// \brief Adds a new AutoscanDirectory to the list.
    ///
    /// The scanID of the directory is invalidated and set to
    /// the index in the AutoscanList.
    ///
    /// \param dir AutoscanDirectory to add to the list.
    /// \return scanID of the newly added AutoscanDirectory
    int add(const std::shared_ptr<AutoscanDirectory>& dir);

    std::shared_ptr<AutoscanDirectory> get(const fs::path& location) const;

    std::size_t getEditSize() const;

    std::size_t size() const { return list.size(); }

    /// \brief removes the AutoscanDirectory given by its scan ID
    void remove(std::size_t id, bool edit = false);

    /// \brief removes the AutoscanDirectory if it is a subdirectory of a given location.
    /// \param parent parent directory.
    /// \return AutoscanList of removed directories, where each directory object in the list is a copy and not the original reference.
    std::shared_ptr<AutoscanManager> removeIfSubdir(const fs::path& parent);

    /// \brief Send notification for each directory that is stored in the list.
    ///
    /// \param sub instance of the class that will receive the notifications.
    void notifyAll(Timer::Subscriber* sub);

    /// \brief updates the last_modified data for all AutoscanDirectories.
    void updateLMinDB();

    /// \brief returns a copy of the autoscan list in the form of an array
    std::vector<std::shared_ptr<AutoscanDirectory>> getArrayCopy() const;

    /// \brief Get an AutoscanDirectory given by location on disk from the watch list.
    const AutoscanDirectory& getAutoscanDirectory(const fs::path& location) const;

    /// \brief returns an array of all autoscan directories
    std::vector<std::shared_ptr<AutoscanDirectory>> getAutoscanDirectories() const;

    /// \brief Removes an AutoscanDirectrory (found by scanID) from the watch list.
    void removeAutoscanDirectory(const std::shared_ptr<AutoscanDirectory>& adir);

    /// \brief Update autoscan parameters for an existing autoscan directory
    /// or add a new autoscan directory
    void setAutoscanDirectory(const std::shared_ptr<AutoscanDirectory>& dir);

    /// \brief handles the removal of a persistent autoscan directory
    void handlePeristentAutoscanRemove(const std::shared_ptr<AutoscanDirectory>& adir);

    /// \brief handles the recreation of a persistent autoscan directory
    void handlePersistentAutoscanRecreate(const std::shared_ptr<AutoscanDirectory>& adir);

private:
    std::size_t origSize {};
    std::map<std::size_t, std::shared_ptr<AutoscanDirectory>> indexMap;



    mutable std::recursive_mutex mutex;
    using AutoLock = std::scoped_lock<std::recursive_mutex>;

    std::vector<std::shared_ptr<AutoscanDirectory>> list;
    int _add(const std::shared_ptr<AutoscanDirectory>& dir);

    struct KeyHasher
    {
        std::size_t operator()(const fs::path& k) const
        {
            using std::filesystem::hash_value;

            return hash_value(k);
        }
    };

    std::unordered_map<fs::path, AutoscanDirectory, KeyHasher> autoscans;
#ifdef HAVE_INOTIFY
    std::unique_ptr<AutoscanInotify> inotify;
#endif

    std::shared_ptr<Config> config;
    std::shared_ptr<Database> database;
};

#endif //__AUTOSCAN_LIST_H__
