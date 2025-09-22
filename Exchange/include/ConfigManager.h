#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <vector>

#include "OrderUtils.h"

namespace Exchange {

class ConfigManager {
public:
    static ConfigManager& instance();

    const std::vector<Symbol>& getSupportedSymbols() const;

private:
    ConfigManager();
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    std::vector<Symbol> supportedSymbols_;
};

} // namespace Exchange

#endif // CONFIG_MANAGER_H

