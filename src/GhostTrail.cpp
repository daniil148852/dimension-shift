#include "GhostTrail.hpp"

// --- GhostTrailNode ---

GhostTrailNode* GhostTrailNode::create(GhostAttempt const& attempt, int ghostIndex) {
    auto ret = new GhostTrailNode();
    if (ret->init(attempt, ghostIndex)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GhostTrailNode::init(GhostAttempt const& attempt, int ghostIndex) {
    if (!CCNode::init()) return false;

    m_attempt = attempt;
    m_ghostIndex = ghostIndex;
    m_currentFrameIndex = 0;

    // Create ghost sprite - use player icon as base
    m_ghostSprite = cocos2d::CCSprite::create("player_01_001.png");
    if (!m_ghostSprite) {
        m_ghostSprite = cocos2d::CCSprite::create("GJ_square01.png");
    }

    if (m_ghostSprite) {
        m_ghostSprite->setColor(getGhostColor());
        float opacity = Mod::get()->getSettingValue<double>("ghost-opacity");
        m_ghostSprite->setOpacity(static_cast<GLubyte>(opacity * 255.f));
        m_ghostSprite->setScale(0.8f);

        // Add glow effect
        auto glow = cocos2d::CCSprite::create("GJ_square01.png");
        if (glow) {
            glow->setColor(getGhostColor());
            glow->setOpacity(40);
            glow->setScale(1.5f);
            glow->setBlendFunc({GL_SRC_ALPHA, GL_ONE}); // Additive blending
            m_ghostSprite->addChild(glow, -1);
        }

        this->addChild(m_ghostSprite);
    }

    return true;
}

cocos2d::ccColor3B GhostTrailNode::getGhostColor() const {
    // Each ghost gets a unique spectral color
    float hue = fmod(m_ghostIndex * 47.f + 180.f, 360.f);
    float h = hue / 60.f;
    float c = 0.7f;
    float x = c * (1.f - fabs(fmod(h, 2.f) - 1.f));

    float r = 0, g = 0, b = 0;
    if (h < 1) { r = c; g = x; }
    else if (h < 2) { r = x; g = c; }
    else if (h < 3) { g = c; b = x; }
    else if (h < 4) { g = x; b = c; }
    else if (h < 5) { r = x; b = c; }
    else { r = c; b = x; }

    return {
        static_cast<GLubyte>((r + 0.3f) * 255),
        static_cast<GLubyte>((g + 0.3f) * 255),
        static_cast<GLubyte>((b + 0.3f) * 255)
    };
}

void GhostTrailNode::updateGhost(float currentTime) {
    if (!m_ghostSprite || m_attempt.frames.empty()) return;

    // Find the right frame for current time
    while (m_currentFrameIndex < static_cast<int>(m_attempt.frames.size()) - 1 &&
           m_attempt.frames[m_currentFrameIndex + 1].timestamp <= currentTime) {
        m_currentFrameIndex++;
    }

    if (m_currentFrameIndex >= static_cast<int>(m_attempt.frames.size())) {
        m_ghostSprite->setVisible(false);
        return;
    }

    auto& frame = m_attempt.frames[m_currentFrameIndex];

    // Interpolate between frames if possible
    cocos2d::CCPoint pos = frame.position;
    float rot = frame.rotation;

    if (m_currentFrameIndex < static_cast<int>(m_attempt.frames.size()) - 1) {
        auto& nextFrame = m_attempt.frames[m_currentFrameIndex + 1];
        float frameDelta = nextFrame.timestamp - frame.timestamp;
        if (frameDelta > 0.f) {
            float t = (currentTime - frame.timestamp) / frameDelta;
            t = std::max(0.f, std::min(1.f, t));
            pos.x = frame.position.x + (nextFrame.position.x - frame.position.x) * t;
            pos.y = frame.position.y + (nextFrame.position.y - frame.position.y) * t;
            rot = frame.rotation + (nextFrame.rotation - frame.rotation) * t;
        }
    }

    m_ghostSprite->setPosition(pos);
    m_ghostSprite->setRotation(rot);
    m_ghostSprite->setVisible(true);
    m_ghostSprite->setFlipY(frame.isFlipped);

    // Slight floating animation
    float wobble = std::sin(currentTime * 3.f + m_ghostIndex * 1.5f) * 3.f;
    m_ghostSprite->setPositionY(pos.y + wobble);

    // Fade near death point
    float deathPercent = m_attempt.deathPercent;
    float framePercent = frame.timestamp; // approximate
    float nearDeath = 1.f - std::max(0.f, std::min(1.f, (deathPercent - framePercent) / 5.f));
    if (nearDeath > 0.5f) {
        float flickerOpacity = Mod::get()->getSettingValue<double>("ghost-opacity") * 255.f;
        flickerOpacity *= (1.f - nearDeath * 0.5f);
        // Flicker effect near death
        flickerOpacity *= (0.7f + 0.3f * std::sin(currentTime * 15.f));
        m_ghostSprite->setOpacity(static_cast<GLubyte>(flickerOpacity));
    }
}

void GhostTrailNode::setGhostVisible(bool visible) {
    if (m_ghostSprite) m_ghostSprite->setVisible(visible);
}

// --- GhostTrailManager ---

GhostTrailManager* GhostTrailManager::create() {
    auto ret = new GhostTrailManager();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool GhostTrailManager::init() {
    if (!CCNode::init()) return false;
    return true;
}

void GhostTrailManager::setupGhosts(cocos2d::CCLayer* gameLayer) {
    clearGhosts();
    m_gameLayer = gameLayer;

    if (!Mod::get()->getSettingValue<bool>("ghost-trail")) return;

    auto& attempts = DimensionManager::get()->getGhostAttempts();
    for (int i = 0; i < static_cast<int>(attempts.size()); i++) {
        auto ghost = GhostTrailNode::create(attempts[i], i);
        if (ghost) {
            m_ghosts.push_back(ghost);
            this->addChild(ghost, 10);
        }
    }
}

void GhostTrailManager::updateGhosts(float currentTime) {
    if (!Mod::get()->getSettingValue<bool>("ghost-trail")) return;

    for (auto ghost : m_ghosts) {
        ghost->updateGhost(currentTime);
    }
}

void GhostTrailManager::clearGhosts() {
    for (auto ghost : m_ghosts) {
        ghost->removeFromParent();
    }
    m_ghosts.clear();
}
