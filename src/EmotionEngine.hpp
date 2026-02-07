#pragma once

#include <Geode/Geode.hpp>
#include "DimensionManager.hpp"

using namespace geode::prelude;

class EmotionEngine {
public:
    static EmotionEngine* get();

    void update(float dt);
    void applyToLayer(cocos2d::CCLayer* layer);

    cocos2d::ccColor3B getCurrentTint() const;
    cocos2d::ccColor3B getBackgroundOverlay() const;
    float getScreenShakeIntensity() const;
    float getVignetteIntensity() const;

private:
    EmotionEngine() = default;

    EmotionState m_currentEmotion = EmotionState::Neutral;
    EmotionState m_targetEmotion = EmotionState::Neutral;
    float m_transitionProgress = 0.f;
    float m_emotionIntensity = 0.f;

    cocos2d::ccColor3B m_currentColor = {255, 255, 255};
    cocos2d::ccColor3B m_targetColor = {255, 255, 255};
    float m_hueShift = 0.f;
    float m_saturationMod = 1.f;

    cocos2d::ccColor3B lerpColor(cocos2d::ccColor3B a, cocos2d::ccColor3B b, float t) const;
    cocos2d::ccColor3B getEmotionColor(EmotionState state) const;
};
