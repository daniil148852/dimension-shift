#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class DimensionSettingsPopup : public geode::Popup<> {
protected:
    bool setup() override;
    void createToggle(const char* label, const char* settingKey, float yPos);

public:
    static DimensionSettingsPopup* create();
};
