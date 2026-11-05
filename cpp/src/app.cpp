#include "app.hpp"

std::vector<AppInfo>& appRegistry() {
    static std::vector<AppInfo> reg;
    return reg;
}

void registerApp(const std::string& name, std::function<std::unique_ptr<App>()> make) {
    appRegistry().push_back({name, std::move(make)});
}
