//
// Created by ian on 3/19/21.
//

#include "autoscan_manager.h"

#include <utility>

#include "autoscan_inotify.h"
#include "config/config.h"
#include "database/database.h"

AutoscanManager::AutoscanManager(std::shared_ptr<Database> database_, std::shared_ptr<Config> config_) : database(std::move(database_)), config(std::move(config_))
{
    autoscans = std::make_unique<std::vector<std::shared_ptr<AutoscanDirectory>>>();

    auto config_timed_list = config->getAutoscanListOption(CFG_IMPORT_AUTOSCAN_TIMED_LIST);
    for (size_t i = 0; i < config_timed_list->size(); i++) {
        auto dir = config_timed_list->get(i);
        if (dir != nullptr) {
            fs::path path = dir->getLocation();
            if (fs::is_directory(path)) {
                dir->setObjectID(ensurePathExistence(path));
            }
        }
    }

    database->updateAutoscanList(ScanMode::Timed, config_timed_list);
    auto autoscan_timed = database->getAutoscanList(ScanMode::Timed);

#ifdef HAVE_INOTIFY
    inotify = std::make_unique<AutoscanInotify>(self);

    if (config->getBoolOption(CFG_IMPORT_AUTOSCAN_USE_INOTIFY)) {
        auto config_inotify_list = config->getAutoscanListOption(CFG_IMPORT_AUTOSCAN_INOTIFY_LIST);
        for (size_t i = 0; i < config_inotify_list->size(); i++) {
            auto dir = config_inotify_list->get(i);
            if (dir != nullptr) {
                fs::path path = dir->getLocation();
                if (fs::is_directory(path)) {
                    dir->setObjectID(ensurePathExistence(path));
                }
            }
        }

        database->updateAutoscanList(ScanMode::INotify, config_inotify_list);
        autoscan_inotify = database->getAutoscanList(ScanMode::INotify);
    } else {
        // make an empty list so we do not have to do extra checks on shutdown
        autoscan_inotify = std::make_shared<AutoscanList>(database);
    }

    // Start INotify thread
    inotify->run();
#endif
}
