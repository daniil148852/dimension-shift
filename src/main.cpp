#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "DimensionManager.hpp"
#include "EmotionEngine.hpp"
#include "GhostTrail.hpp"
#include "TimeEcho.hpp"
#include "ChromaWorld.hpp"
#include "SettingsPopup.hpp"

using namespace geode::prelude;

// ============================================
// PlayLayer hooks - Core gameplay integration
// ============================================

class $modify(DimensionPlayLayer, PlayLayer) {
    struct Fields {
        GhostTrailManager* ghostManager = nullptr;
        TimeEchoEffect* timeEcho = nullptr;
        ChromaWorld* chromaWorld = nullptr;
        float gameTime = 0.f;
        float recordInterval = 0.f;
        bool initialized = false;
        int levelID = 0;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (!Mod::get()->getSettingValue<bool>("enabled")) return true;

        auto f = m_fields.self();

        // Track level changes
        int newLevelID = level->m_levelID.value();
        if (newLevelID != f->levelID) {
            DimensionManager::get()->clearForNewLevel();
            f->levelID = newLevelID;
        }

        // Initialize subsystems
        f->ghostManager = GhostTrailManager::create();
        this->addChild(f->ghostManager, 15);

        f->timeEcho = TimeEchoEffect::create();
        this->addChild(f->timeEcho, 1);

        f->chromaWorld = ChromaWorld::create();
        this->addChild(f->chromaWorld, 900);

        // Setup vignette and rhythm pulse
        f->chromaWorld->createVignette(this);
        f->chromaWorld->createRhythmPulse(this);

        // Try to detect BPM from level settings
        // GD levels have songOffset and speed settings we can approximate from
        float bpm = 120.f; // default
        if (level->m_songID > 0 || level->m_audioTrack > 0) {
            // Rough BPM estimation - could be improved with audio analysis
            bpm = 130.f; // Common GD song BPM range
        }
        f->chromaWorld->setCurrentBPM(bpm);

        f->initialized = true;
        f->gameTime = 0.f;

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        if (!Mod::get()->getSettingValue<bool>("enabled")) return;

        auto f = m_fields.self();
        auto dm = DimensionManager::get();

        // Record the death if we had any progress
        if (f->gameTime > 0.2f) {
            float percent = this->getCurrentPercent();
            dm->recordDeath(m_player1->getPosition(), percent);
            dm->finalizeAttempt(percent);

            // Create dimension fracture at death point
            if (f->chromaWorld) {
                int deathCount = dm->getDeathCountAt(m_player1->getPosition(), 50.f);
                f->chromaWorld->createDimensionFracture(m_player1->getPosition(), deathCount);
            }

            // Clear non-persistent fractures
            if (!Mod::get()->getSettingValue<bool>("fracture-persist")) {
                // Fractures reset per attempt (handled in ChromaWorld)
            }
        }

        f->gameTime = 0.f;
        f->recordInterval = 0.f;
        dm->reset();

        // Rebuild ghosts with new data
        if (f->ghostManager) {
            f->ghostManager->setupGhosts(this);
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!Mod::get()->getSettingValue<bool>("enabled")) return;

        auto f = m_fields.self();
        if (!f->initialized) return;

        f->gameTime += dt;
        f->recordInterval += dt;

        auto dm = DimensionManager::get();

        // Record player position for ghost trail (every 1/30th of a second)
        if (f->recordInterval >= 1.f / 30.f) {
            dm->recordFrame(m_player1, f->gameTime);
            f->recordInterval = 0.f;
        }

        // Update emotion engine
        EmotionEngine::get()->update(dt);
        EmotionEngine::get()->applyToLayer(this);

        // Update ghost trails
        if (f->ghostManager) {
            f->ghostManager->updateGhosts(f->gameTime);
        }

        // Apply time echo distortion to visible objects
        if (f->timeEcho && m_objectLayer) {
            float cameraX = m_gameState.m_cameraPosition.x;
            f->timeEcho->applyToObjects(m_objectLayer->getChildren(), cameraX);
        }

        // Screen shake on frustration
        float shakeIntensity = EmotionEngine::get()->getScreenShakeIntensity();
        if (shakeIntensity > 0.1f) {
            float shakeX = (static_cast<float>(rand() % 100) / 100.f - 0.5f) * shakeIntensity;
            float shakeY = (static_cast<float>(rand() % 100) / 100.f - 0.5f) * shakeIntensity;
            auto originalPos = this->getPosition();
            this->setPosition({originalPos.x + shakeX, originalPos.y + shakeY});
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        if (!Mod::get()->getSettingValue<bool>("enabled")) return;

        // Trigger triumph state
        auto dm = DimensionManager::get();
        dm->finalizeAttempt(100.f);

        // Epic triumph visual burst
        auto f = m_fields.self();
        if (f->chromaWorld) {
            // Create burst of fractures in a circle (but as celebration, not death)
            auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
            auto center = ccp(winSize.width / 2, winSize.height / 2);

            for (int i = 0; i < 8; i++) {
                float angle = (static_cast<float>(i) / 8.f) * 360.f;
                float rad = angle * M_PI / 180.f;
                auto pos = ccp(
                    center.x + std::cos(rad) * 100.f,
                    center.y + std::sin(rad) * 100.f
                );
                f->chromaWorld->createDimensionFracture(pos, 1);
            }
        }
    }
};

// ============================================
// PauseLayer hook - Add settings button
// ============================================

class $modify(DimensionPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        if (!Mod::get()->getSettingValue<bool>("enabled")) return;

        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

        auto btn = CCMenuItemSpriteExtra::create(
            cocos2d::CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this,
            menu_selector(DimensionPauseLayer::onDimensionSettings)
        );
        btn->setScale(0.7f);

        auto menu = cocos2d::CCMenu::create();
        menu->setPosition({winSize.width - 40.f, 40.f});
        menu->addChild(btn);
        this->addChild(menu, 100);

        // Add dimension label
        auto label = cocos2d::CCLabelBMFont::create("Dimension", "bigFont.fnt");
        label->setScale(0.25f);
        label->setPosition({winSize.width - 40.f, 20.f});
        label->setOpacity(150);
        this->addChild(label, 100);
    }

    void onDimensionSettings(CCObject* sender) {
        DimensionSettingsPopup::create()->show();
    }
};

// ============================================
// MenuLayer hook - Show stats on main menu
// ============================================

class $modify(DimensionMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (!Mod::get()->getSettingValue<bool>("enabled")) return true;

        auto dm = DimensionManager::get();

        if (dm->getTotalDeaths() > 0) {
            // Show a subtle "Reality Distortion" indicator
            auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

            std::string statusText;
            switch (dm->calculateEmotion()) {
                case EmotionState::Neutral:       statusText = "Reality: Stable"; break;
                case EmotionState::Frustration:    statusText = "Reality: Fracturing"; break;
                case EmotionState::Hope:           statusText = "Reality: Blooming"; break;
                case EmotionState::Determination:  statusText = "Reality: Hardening"; break;
                case EmotionState::Triumph:        statusText = "Reality: Transcendent"; break;
                case EmotionState::Zen:            statusText = "Reality: At Peace"; break;
            }

            auto label = cocos2d::CCLabelBMFont::create(statusText.c_str(), "chatFont.fnt");
            label->setPosition({winSize.width / 2, 20.f});
            label->setScale(0.5f);
            label->setOpacity(100);
            label->setColor(EmotionEngine::get()->getCurrentTint());
            this->addChild(label, 100);

            // Gentle fade animation
            auto fadeIn = cocos2d::CCFadeIn::create(2.f);
            auto fadeOut = cocos2d::CCFadeOut::create(2.f);
            auto delay = cocos2d::CCDelayTime::create(3.f);
            auto seq = cocos2d::CCSequence::create(fadeIn, delay, fadeOut, nullptr);
            label->runAction(cocos2d::CCRepeatForever::create(
                dynamic_cast<cocos2d::CCSequence*>(
                    cocos2d::CCSequence::create(fadeIn, delay, fadeOut, delay->copy(), nullptr)
                )
            ));
        }

        return true;
    }
};
