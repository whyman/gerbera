//
// Created by ian on 3/19/21.
//

#ifndef __AUTOSCAN_MANAGER_H__
#define __AUTOSCAN_MANAGER_H__

#include <memory>
#include <vector>

#include "autoscan.h"

class Config;
class Database;

class AutoscanManager {
    AutoscanManager(std::shared_ptr<Database> database_, std::shared_ptr<Config> config_);

protected:
    std::shared_ptr<Database> database;
    std::shared_ptr<Config> config;

    std::unique_ptr<std::vector<std::shared_ptr<AutoscanDirectory>>> autoscans;
};

#endif //__AUTOSCAN_MANAGER_H__
