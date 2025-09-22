#include "ConfigManager.h"

namespace Exchange {

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager() {
    supportedSymbols_.reserve(10);
    supportedSymbols_.emplace_back("AAPL"_sym);
    supportedSymbols_.emplace_back("GOOGL"_sym);
    supportedSymbols_.emplace_back("MSFT"_sym);
    supportedSymbols_.emplace_back("AMZN"_sym);
    supportedSymbols_.emplace_back("META"_sym);
    supportedSymbols_.emplace_back("NVDA"_sym);
    supportedSymbols_.emplace_back("TSLA"_sym);
    supportedSymbols_.emplace_back("NFLX"_sym);
    supportedSymbols_.emplace_back("JPM"_sym);
    supportedSymbols_.emplace_back("V"_sym);
}

const std::vector<Symbol>& ConfigManager::getSupportedSymbols() const {
    return supportedSymbols_;
}

} // namespace Exchange

