#include "TimeEcho.hpp"
#include <cmath>

TimeEchoEffect* TimeEchoEffect::create() {
    auto ret = new TimeEchoEffect();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool TimeEchoEffect::init() {
    if (!CCNode::init()) return false;
    this->scheduleUpdate();
    return true;
}

void TimeEchoEffect::update(float dt) {
    m_time += dt;
}

void TimeEchoEffect::applyToObjects(cocos2d::CCArray* objects, float cameraX) {
    if (!objects) return;
    if (!Mod::get()->getSettingValue<bool>("time-echo")) return;

    auto dm = DimensionManager::get();
    auto& deathPoints = dm->getDeathPoints();

    if (deathPoints.empty()) return;

    float viewWidth = cocos2d::CCDirector::sharedDirector()->getWinSize().width;

    for (int i = 0; i < objects->count(); i++) {
        auto obj = static_cast<cocos2d::CCNode*>(objects->objectAtIndex(i));
        if (!obj) continue;

        auto objPos = obj->getPosition();

        // Only process objects near camera
        if (objPos.x < cameraX - 100.f || objPos.x > cameraX + viewWidth + 100.f) {
            continue;
        }

        // Check distance to death points
        for (auto& dp : deathPoints) {
            float dist = objPos.getDistance(dp.position);
            float effectRadius = 150.f + dp.deathCount * 30.f;

            if (dist < effectRadius) {
                distortObject(obj, dp.deathCount, dist / effectRadius);
            }
        }
    }
}

void TimeEchoEffect::distortObject(cocos2d::CCNode* obj, int deathCount, float normalizedDist) {
    float intensity = (1.f - normalizedDist) * std::min(1.f, deathCount / 10.f);

    // Positional wobble - objects vibrate slightly
    float wobbleX = std::sin(m_time * 8.f + obj->getPositionX() * 0.1f) * intensity * 3.f;
    float wobbleY = std::cos(m_time * 6.f + obj->getPositionY() * 0.1f) * intensity * 3.f;

    // Store original position if not stored yet
    if (obj->getUserData() == nullptr) {
        // Use a small offset applied each frame - it self-corrects
        // because we're applying relative to current pos through visual transform
    }

    // Scale distortion - slight breathing effect
    float scaleWobble = 1.f + std::sin(m_time * 4.f + obj->getPositionX()) * intensity * 0.08f;
    obj->setScale(obj->getScale() > 0 ? std::abs(obj->getScale()) * scaleWobble : scaleWobble);

    // Rotation micro-tremor
    float rotTremor = std::sin(m_time * 12.f + obj->getPositionY() * 0.05f) * intensity * 5.f;

    // Apply visual offset using the node's additional transform
    // We apply this as a skew to avoid messing with position (which affects gameplay)
    obj->setSkewX(wobbleX);
    obj->setSkewY(wobbleY);
    obj->setRotation(obj->getRotation() + rotTremor * 0.1f);

    // Color distortion - objects near death zones get a red/purple tint
    auto sprite = dynamic_cast<cocos2d::CCSprite*>(obj);
    if (sprite) {
        float redShift = intensity * 0.3f;
        auto color = sprite->getColor();
        GLubyte r = std::min(255, static_cast<int>(color.r + redShift * 80));
        GLubyte g = std::max(0, static_cast<int>(color.g - redShift * 30));
        GLubyte b = static_cast<int>(color.b + redShift * 40);
        sprite->setColor({r, static_cast<GLubyte>(g), static_cast<GLubyte>(b)});
    }

    // Opacity flicker for heavy death zones
    if (deathCount > 5) {
        float flicker = 0.85f + 0.15f * std::sin(m_time * 20.f + obj->getPositionX());
        obj->setOpacity(static_cast<GLubyte>(obj->getOpacity() * flicker));
    }
}
