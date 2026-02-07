#include "SettingsPopup.hpp"
#include "DimensionManager.hpp"
#include "EmotionEngine.hpp"

DimensionSettingsPopup* DimensionSettingsPopup::create() {
    auto ret = new DimensionSettingsPopup();
    if (ret->initAnchored(380.f, 300.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DimensionSettingsPopup::setup() {
    this->setTitle("Dimension Shift");

    auto winSize = this->m_mainLayer->getContentSize();

    // Stats display
    auto dm = DimensionManager::get();
    auto emotion = EmotionEngine::get();

    std::string emotionName;
    switch (dm->calculateEmotion()) {
        case EmotionState::Neutral:       emotionName = "Neutral"; break;
        case EmotionState::Frustration:   emotionName = "Frustration"; break;
        case EmotionState::Hope:          emotionName = "Hope"; break;
        case EmotionState::Determination: emotionName = "Determination"; break;
        case EmotionState::Triumph:       emotionName = "Triumph"; break;
        case EmotionState::Zen:           emotionName = "Zen"; break;
    }

    auto statsLabel = cocos2d::CCLabelBMFont::create(
        fmt::format(
            "Deaths: {} | Best: {:.1f}% | Emotion: {} | Ghosts: {}",
            dm->getTotalDeaths(),
            dm->getBestPercent(),
            emotionName,
            dm->getGhostAttempts().size()
        ).c_str(),
        "chatFont.fnt"
    );
    statsLabel->setScale(0.55f);
    statsLabel->setPosition({winSize.width / 2, winSize.height - 55.f});
    this->m_mainLayer->addChild(statsLabel);

    // Frustration bar
    auto frustLabel = cocos2d::CCLabelBMFont::create("Frustration:", "bigFont.fnt");
    frustLabel->setScale(0.35f);
    frustLabel->setPosition({80.f, winSize.height - 80.f});
    this->m_mainLayer->addChild(frustLabel);

    float frustration = dm->getFrustrationLevel();
    auto barBg = cocos2d::CCLayerColor::create({50, 50, 50, 200}, 200.f, 12.f);
    barBg->setPosition({140.f, winSize.height - 86.f});
    this->m_mainLayer->addChild(barBg);

    cocos2d::ccColor4B barColor;
    if (frustration < 0.3f) barColor = {80, 200, 80, 255};
    else if (frustration < 0.6f) barColor = {220, 180, 40, 255};
    else barColor = {220, 50, 50, 255};

    auto barFill = cocos2d::CCLayerColor::create(barColor, 200.f * frustration, 12.f);
    barFill->setPosition({140.f, winSize.height - 86.f});
    this->m_mainLayer->addChild(barFill);

    // Toggles
    float y = winSize.height - 120.f;
    createToggle("Ghost Trail", "ghost-trail", y); y -= 32.f;
    createToggle("Time Echo", "time-echo", y); y -= 32.f;
    createToggle("Emotion Engine", "emotion-engine", y); y -= 32.f;
    createToggle("Rhythm Pulse", "rhythm-pulse", y); y -= 32.f;
    createToggle("Dimension Fractures", "dimension-fracture", y); y -= 32.f;
    createToggle("Persistent Fractures", "fracture-persist", y);

    // Current emotion color swatch
    auto emotionSwatch = cocos2d::CCLayerColor::create(
        {emotion->getCurrentTint().r, emotion->getCurrentTint().g, emotion->getCurrentTint().b, 255},
        30.f, 30.f
    );
    emotionSwatch->setPosition({winSize.width - 50.f, winSize.height - 55.f});
    this->m_mainLayer->addChild(emotionSwatch);

    return true;
}

void DimensionSettingsPopup::createToggle(const char* label, const char* settingKey, float yPos) {
    auto winSize = this->m_mainLayer->getContentSize();

    auto text = cocos2d::CCLabelBMFont::create(label, "bigFont.fnt");
    text->setScale(0.4f);
    text->setAnchorPoint({0, 0.5f});
    text->setPosition({30.f, yPos});
    this->m_mainLayer->addChild(text);

    bool currentVal = Mod::get()->getSettingValue<bool>(settingKey);
    std::string settingStr = settingKey;

    auto toggler = CCMenuItemToggler::createWithStandardSprites(
        this,
        menu_selector(DimensionSettingsPopup::onClose), // placeholder
        0.6f
    );
    toggler->toggle(currentVal);
    toggler->setPosition({winSize.width - 50.f, yPos});

    // Use callback to update setting
    toggler->setClickable(true);

    auto menu = cocos2d::CCMenu::create();
    menu->setPosition({0, 0});
    menu->addChild(toggler);
    this->m_mainLayer->addChild(menu);
}
