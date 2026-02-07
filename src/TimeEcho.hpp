#pragma once

#include <Geode/Geode.hpp>
#include "DimensionManager.hpp"

using namespace geode::prelude;

class TimeEchoEffect : public cocos2d::CCNode {
public:
    static TimeEchoEffect* create();
    bool init() override;

    void applyToObjects(cocos2d::CCArray* objects, float cameraX);
    void update(float dt) override;

private:
    float m_time = 0.f;

    void distortObject(cocos2d::CCNode* obj, int deathCount, float dist);
};
