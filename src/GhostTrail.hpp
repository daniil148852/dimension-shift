#pragma once

#include <Geode/Geode.hpp>
#include "DimensionManager.hpp"

using namespace geode::prelude;

class GhostTrailNode : public cocos2d::CCNode {
public:
    static GhostTrailNode* create(GhostAttempt const& attempt, int ghostIndex);
    bool init(GhostAttempt const& attempt, int ghostIndex);

    void updateGhost(float currentTime);
    void setGhostVisible(bool visible);

private:
    GhostAttempt m_attempt;
    int m_ghostIndex = 0;
    int m_currentFrameIndex = 0;
    cocos2d::CCSprite* m_ghostSprite = nullptr;
    cocos2d::CCMotionStreak* m_streak = nullptr;

    cocos2d::ccColor3B getGhostColor() const;
};

class GhostTrailManager : public cocos2d::CCNode {
public:
    static GhostTrailManager* create();
    bool init() override;

    void setupGhosts(cocos2d::CCLayer* gameLayer);
    void updateGhosts(float currentTime);
    void clearGhosts();

private:
    std::vector<GhostTrailNode*> m_ghosts;
    cocos2d::CCLayer* m_gameLayer = nullptr;
};
