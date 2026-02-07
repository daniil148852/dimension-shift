#include "EmotionEngine.hpp"
#include <cmath>

EmotionEngine* EmotionEngine::get() {
    static EmotionEngine instance;
    return &instance;
}

cocos2d::ccColor3B EmotionEngine::lerpColor(cocos2d::ccColor3B a, cocos2d::ccColor3B b, float t) const {
    t = std::max(0.f, std::min(1.f, t));
    return {
        static_cast<GLubyte>(a.r + (b.r - a.r) * t),
        static_cast<GLubyte>(a.g + (b.g - a.g) * t),
        static_cast<GLubyte>(a.b + (b.b - a.b) * t)
    };
}

cocos2d::ccColor3B EmotionEngine::getEmotionColor(EmotionState state) const {
    switch (state) {
        case EmotionState::Neutral:       return {200, 210, 230};
        case EmotionState::Frustration:   return {180, 40, 60};
        case EmotionState::Hope:          return {80, 200, 120};
        case EmotionState::Determination: return {220, 140, 40};
        case EmotionState::Triumph:       return {100, 220, 255};
        case EmotionState::Zen:           return {160, 140, 220};
        default:                          return {255, 255, 255};
    }
}

void EmotionEngine::update(float dt) {
    auto dm = DimensionManager::get();
    m_targetEmotion = dm->calculateEmotion();

    if (m_targetEmotion != m_currentEmotion) {
        m_transitionProgress += dt * 0.3f; // Slow, smooth transitions
        if (m_transitionProgress >= 1.f) {
            m_currentEmotion = m_targetEmotion;
            m_transitionProgress = 0.f;
        }
    }

    m_targetColor = getEmotionColor(m_targetEmotion);
    cocos2d::ccColor3B fromColor = getEmotionColor(m_currentEmotion);
    m_currentColor = lerpColor(fromColor, m_targetColor, m_transitionProgress);

    // Frustration causes subtle hue oscillation
    if (m_currentEmotion == EmotionState::Frustration ||
        m_targetEmotion == EmotionState::Frustration) {
        float frustration = dm->getFrustrationLevel();
        m_hueShift = std::sin(static_cast<float>(clock()) * 0.002f) * frustration * 30.f;
    } else {
        m_hueShift *= 0.95f; // decay
    }

    // Triumph causes rainbow cycling
    if (m_currentEmotion == EmotionState::Triumph) {
        float time = static_cast<float>(clock()) * 0.001f;
        float r = (std::sin(time) * 0.5f + 0.5f) * 255.f;
        float g = (std::sin(time + 2.094f) * 0.5f + 0.5f) * 255.f;
        float b = (std::sin(time + 4.189f) * 0.5f + 0.5f) * 255.f;
        m_currentColor = lerpColor(m_currentColor,
            {static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b)}, 0.3f);
    }

    m_emotionIntensity = std::max(dm->getFrustrationLevel(), 1.f - dm->getProgressMomentum());
}

void EmotionEngine::applyToLayer(cocos2d::CCLayer* layer) {
    if (!layer) return;
    if (!Mod::get()->getSettingValue<bool>("emotion-engine")) return;

    // Apply color tint to background
    auto children = layer->getChildren();
    if (!children) return;

    for (int i = 0; i < children->count(); i++) {
        auto node = static_cast<cocos2d::CCNode*>(children->objectAtIndex(i));
        if (!node) continue;

        // Apply subtle tint - blend with existing color
        auto sprite = dynamic_cast<cocos2d::CCSprite*>(node);
        if (sprite && node->getTag() == -1) { // Background elements
            auto origColor = sprite->getColor();
            auto tinted = lerpColor(origColor, m_currentColor, 0.15f);
            sprite->setColor(tinted);
        }
    }
}

cocos2d::ccColor3B EmotionEngine::getCurrentTint() const {
    return m_currentColor;
}

cocos2d::ccColor3B EmotionEngine::getBackgroundOverlay() const {
    auto color = m_currentColor;
    // Make it more subtle for overlay
    return lerpColor({128, 128, 128}, color, 0.4f);
}

float EmotionEngine::getScreenShakeIntensity() const {
    if (m_currentEmotion == EmotionState::Frustration) {
        return DimensionManager::get()->getFrustrationLevel() * 2.f;
    }
    return 0.f;
}

float EmotionEngine::getVignetteIntensity() const {
    switch (m_currentEmotion) {
        case EmotionState::Frustration:  return 0.4f;
        case EmotionState::Triumph:      return 0.1f;
        case EmotionState::Determination: return 0.25f;
        default: return 0.f;
    }
}
