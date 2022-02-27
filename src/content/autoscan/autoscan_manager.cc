/*GRB*

    Gerbera - https://gerbera.io/

    autoscan_list.cc - this file is part of Gerbera.

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

/// \file autoscan_list.cc
#include "autoscan_manager.h"

#include "autoscan_directory.h"
#include "database/database.h"
#include "content/file_processor.h"
#include "timed_autoscan_directory.h"

// TODO: DEFINE CONTENT MANAGER <-> AUTOSCAN MANAGER INTERFACE

AutoscanManager::AutoscanManager(std::shared_ptr<Database> database, std::shared_ptr<Config> config, FileProcessor& fileProcessor)
    : config(std::move(config))
    , database(std::move(database))
    , fileProcessor(fileProcessor)
#ifdef HAVE_INOTIFY
    , inotify(AutoscanInotify(*this, fileProcessor, config->getBoolOption(ATTR_AUTOSCAN_DIRECTORY_HIDDENFILES)))
#endif
{
    log_debug("Starting AutoscanManager!");
    auto configTimedList = config->getAutoscanListOption(CFG_IMPORT_AUTOSCAN_TIMED_LIST);
    for (auto dir : configTimedList) {
        fs::path path = dir.getLocation();
        if (fs::is_directory(path)) {
            dir.setObjectID(fileProcessor.ensurePathExistence(path));
        }
        autoscanMap[dir.getLocation()] = std::move(dir);
    }

#ifdef HAVE_INOTIFY
    auto configInotifyList = config->getAutoscanListOption(CFG_IMPORT_AUTOSCAN_INOTIFY_LIST);
    for (auto&& dir : configInotifyList) {
        fs::path path = dir.getLocation();
        if (fs::is_directory(path)) {
            dir.setObjectID(fileProcessor.ensurePathExistence(path));
        }
        autoscanMap[dir.getLocation()] = dir;
    }
#endif

    log_debug("Loading autoscans from DB...");
    auto autoscan_timed = database->getAutoscanList(ScanMode::Timed);
    for (auto&& dir : autoscan_timed) {
        autoscanMap[dir.getLocation()] = dir;
    }

#ifdef TOMBDEBUG
    log_debug("Autoscans:");
    for (auto[path, dir] : autoscanMap) {
        log_debug("    [{}] {} (via {})", AutoscanDirectory::mapScanMode(dir.getScanMode()), path.string(), dir.getSource());
    }
#endif

    // I dont think we need this as we now own all the autoscan state.
   // database->updateAutoscanList(ScanMode::Timed, configTimedList);

#ifdef HAVE_INOTIFY
    // Start INotify thread
    log_debug("Starting inotify");
    inotify.run();
#endif

    // TODO FIXME
    //autoscan_timed->notifyAll(this);

    for (auto&&[path, dir] : autoscanMap) {
        if (dir.getScanMode() == ScanMode::INotify) {
#ifdef HAVE_INOTIFY
            inotify.monitor(dir);
            auto param = std::make_shared<Timer::Parameter>(Timer::Parameter::timer_param_t::IDAutoscan, dir.getScanID());
            log_debug("Adding one-shot inotify scan");
            // TODO FIXME
            // timer->addTimerSubscriber(this, std::chrono::minutes(1), param, true);
#else
            log_warning("Not monitoring {} via inotify as this build does not support INotify", path.string());
#endif
            break;
        } else if (dir.getScanMode() == ScanMode::Timed) {
            auto timed = dynamic_cast<TimedAutoscanDirectory&>(dir);

            auto param = std::make_shared<Timer::Parameter>(Timer::Parameter::timer_param_t::IDAutoscan, dir.getScanID());
            // TODO FIXME
            log_debug("[[[NOT]]] Adding timed scan with interval {}", timed.getInterval().count());
            // timer->addTimerSubscriber(this, adir->getInterval(), param, false);
            break;
        }
    }
}

void AutoscanManager::updateLMinDB()
{
    AutoLock lock(mutex);
    for (std::size_t i = 0; i < list.size(); i++) {
        log_debug("i: {}", i);
        auto ad = list[i];
        database->updateAutoscanDirectory(ad);
    }
}

int AutoscanManager::add(const std::shared_ptr<AutoscanDirectory>& dir)
{
    AutoLock lock(mutex);
    return _add(dir);
}

int AutoscanManager::_add(const std::shared_ptr<AutoscanDirectory>& dir)
{
    if (std::any_of(list.begin(), list.end(), [loc = dir->getLocation()](auto&& item) { return loc == item->getLocation(); })) {
        throw_std_runtime_error("Attempted to add same autoscan path twice");
    }
    list.push_back(dir);
    indexMap[dir->getScanID()] = dir;

    return dir->getScanID();
}

std::size_t AutoscanManager::getEditSize() const
{
    if (indexMap.empty()) {
        return 0;
    }
    return std::max_element(indexMap.begin(), indexMap.end(), [](auto a, auto b) { return (a.first < b.first); })->first + 1;
}

std::vector<std::shared_ptr<AutoscanDirectory>> AutoscanManager::getArrayCopy() const
{
    AutoLock lock(mutex);

    return list;
}

std::shared_ptr<AutoscanDirectory> AutoscanManager::get(const fs::path& location) const
{
    AutoLock lock(mutex);

    auto it = std::find_if(list.begin(), list.end(), [=](auto&& item) { return location == item->getLocation(); });
    return it != list.end() ? *it : nullptr;
}

void AutoscanManager::remove(std::size_t id, bool edit)
{
    AutoLock lock(mutex);

    if (!edit) {
        if (id >= list.size()) {
            log_debug("No such ID {}!", id);
            return;
        }
        auto dir = list[id];
        dir->invalidate();

        list.erase(list.begin() + id);
        log_debug("ID {} removed!", id);
    } else {
        if (indexMap.find(id) == indexMap.end()) {
            log_debug("No such index ID {}!", id);
            return;
        }
        auto&& dir = indexMap[id];
        auto entry = std::find_if(list.begin(), list.end(), [loc = dir->getScanID()](auto&& item) { return loc == item->getScanID(); });
        dir->invalidate();
        list.erase(entry);

        if (id >= origSize) {
            indexMap.erase(id);
        }
        log_debug("ID {} removed!", id);
    }
}

int AutoscanManager::removeIfSubdir(const fs::path& parent)
{
    AutoLock lock(mutex);
    int removed = 0;
    for (auto it = autoscanMap.begin(); it != autoscanMap.end();) {
        if (it->first < parent) {
            log_debug("Removing {}", it->first.string());

            if (it->second.getScanMode() == ScanMode::INotify) {
                inotify.unmonitor(it->second);
            } else if (it->second.getScanMode() == ScanMode::Timed) {
                // TODO FIXME
                //timer->removeTimerSubscriber(this, rmList->get(i)->getTimerParameter(), true);
            }
            it = autoscanMap.erase(it);
            removed++;
        }
        else {
            it++;
        }
    }
    return removed;
}

void AutoscanManager::notifyAll(Timer::Subscriber* sub)
{
    // TODO FIXME
//    if (!sub)
//        return;
//    AutoLock lock(mutex);
//
//    for (auto&& i : list) {
//        sub->timerNotify(i->getTimerParameter());
//    }
}

const AutoscanDirectory& AutoscanManager::getAutoscanDirectory(const fs::path& location) const
{
    return autoscanMap.at(location);
}

void AutoscanManager::removeAutoscanDirectory(const std::shared_ptr<AutoscanDirectory>& adir)
{
//    if (!adir)
//        throw_std_runtime_error("can not remove autoscan directory - was not an autoscan");
//
//    adir->setTaskCount(-1);
//
//    if (adir->getScanMode() == ScanMode::Timed) {
//        autoscan_timed->remove(adir->getScanID());
//        database->removeAutoscanDirectory(adir);
//        // FIXME TODO
//        //session_manager->containerChangedUI(adir->getObjectID());
//
//        // if 3rd parameter is true: won't fail if scanID doesn't exist
//        //timer->removeTimerSubscriber(this, adir->getTimerParameter(), true);
//        // FIXME TODO
//    }
//#ifdef HAVE_INOTIFY
//    if (adir->getScanMode() == ScanMode::INotify) {
//        autoscan_inotify->remove(adir->getScanID());
//        database->removeAutoscanDirectory(adir);
//        // FIXME TODO
//        // session_manager->containerChangedUI(adir->getObjectID());
//        inotify->unmonitor(adir);
//    }
//#endif
}

void AutoscanManager::handlePeristentAutoscanRemove(const std::shared_ptr<AutoscanDirectory>& adir)
{
    if (adir->isPersistent()) {
        adir->setObjectID(INVALID_OBJECT_ID);
        database->updateAutoscanDirectory(adir);
    } else {
        removeAutoscanDirectory(adir);
    }
}

void AutoscanManager::handlePersistentAutoscanRecreate(const std::shared_ptr<AutoscanDirectory>& adir)
{
//    int id = ensurePathExistence(adir->getLocation());
//    adir->setObjectID(id);
//    database->updateAutoscanDirectory(adir);
}

//void AutoscanManager::setAutoscanDirectory(const std::shared_ptr<AutoscanDirectory>& dir)
//{
    // TODO FIXME
//    // We will have to change this for other scan modes
//    auto original = autoscan_timed->getByObjectID(dir->getObjectID());
//#ifdef HAVE_INOTIFY
//    if (config->getBoolOption(CFG_IMPORT_AUTOSCAN_USE_INOTIFY) && !original)
//        original = autoscan_inotify->getByObjectID(dir->getObjectID());
//#endif
//
//    if (original)
//        dir->setDatabaseID(original->getDatabaseID());
//
//    database->checkOverlappingAutoscans(dir);

//
//    // adding a new autoscan directory
//    if (!original) {
//        if (dir->getObjectID() == CDS_ID_FS_ROOT)
//            dir->setLocation(FS_ROOT_DIRECTORY);
//        else {
//            log_debug("objectID: {}", dir->getObjectID());
//            auto obj = database->loadObject(dir->getObjectID());
//            if (!obj || !obj->isContainer() || obj->isVirtual())
//                throw_std_runtime_error("tried to remove an illegal object (id) from the list of the autoscan directories");
//
//            log_debug("location: {}", obj->getLocation().c_str());
//
//            if (obj->getLocation().empty())
//                throw_std_runtime_error("tried to add an illegal object as autoscan - no location information available");
//
//            dir->setLocation(obj->getLocation());
//        }
//        dir->resetLMT();
//        database->addAutoscanDirectory(dir);
//        if (dir->getScanMode() == ScanMode::Timed) {
//            autoscan_timed->add(dir);
//            reloadLayout();
//            timerNotify(dir->getTimerParameter());
//        }
//#ifdef HAVE_INOTIFY
//        if (config->getBoolOption(CFG_IMPORT_AUTOSCAN_USE_INOTIFY) && (dir->getScanMode() == ScanMode::INotify)) {
//            autoscan_inotify->add(dir);
//            reloadLayout();
//            inotify->monitor(dir);
//        }
//#endif
//        session_manager->containerChangedUI(dir->getObjectID());
//        return;
//    }

// TODO FIXME
//    if (original->getScanMode() == ScanMode::Timed) {
//        timer->removeTimerSubscriber(this, original->getTimerParameter(), true);
//    }
//#ifdef HAVE_INOTIFY
//    if (config->getBoolOption(CFG_IMPORT_AUTOSCAN_USE_INOTIFY) && (original->getScanMode() == ScanMode::INotify)) {
//        inotify->unmonitor(original);
//    }
//#endif

    // TODO FIXME
    //    auto copy = std::make_shared<AutoscanDirectory>();
    //    original->copyTo(copy);
    //
    //    copy->setHidden(dir->getHidden());
    //    copy->setRecursive(dir->isRecursive());
    //    copy->setInterval(dir->getInterval());
    //
    //    if (copy->getScanMode() == ScanMode::Timed) {
    //        autoscan_timed->remove(copy->getScanID());
    //    }
    //#ifdef HAVE_INOTIFY
    //    if (config->getBoolOption(CFG_IMPORT_AUTOSCAN_USE_INOTIFY) && copy->getScanMode() == ScanMode::INotify) {
    //        autoscan_inotify->remove(copy->getScanID());
    //    }
    //#endif
    //
    //    copy->setScanMode(dir->getScanMode());
    //
    //    if (dir->getScanMode() == ScanMode::Timed) {
    //        autoscan_timed->add(copy);
    //        timerNotify(copy->getTimerParameter());
    //    }
    //#ifdef HAVE_INOTIFY
    //    if (config->getBoolOption(CFG_IMPORT_AUTOSCAN_USE_INOTIFY) && dir->getScanMode() == ScanMode::INotify) {
    //        autoscan_inotify->add(copy);
    //        inotify->monitor(copy);
    //    }
    //#endif
    //
    //    database->updateAutoscanDirectory(copy);
    //    if (original->getScanMode() != copy->getScanMode())
    //        session_manager->containerChangedUI(copy->getObjectID());
//}

AutoscanManager::~AutoscanManager()
{
    log_debug("updating last_modified data for autoscan in database...");
    updateLMinDB();
}
