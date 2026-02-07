#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <vector>
#include <unordered_map>

using namespace geode::prelude;

struct DeathPoint {
    cocos2d::CCPoint position;
    int deathCount;
    float timestamp;
};

struct GhostFrame {
    cocos2d::CCPoint position;
    float rotation;
    float timestamp;
    bool isShip;
    bool isBall;
    bool isUfo;
    bool isWave;
    bool isRobot;
    bool isSpider;
    bool isDart;  // swingcopter
    bool isCube;
    bool isFlipped;
    double yVelocity;
};

struct GhostAttempt {
    std::vector<GhostFrame> frames;
    float deathPercent;
    int attemptNumber;
};

enum class EmotionState {
    Neutral,
    Frustration,
    Hope,
    Determination,
    Triumph,
    Zen
};

class DimensionManager {
public:
    static DimensionManager* get();

    void reset();
    void recordFrame(PlayerObject* player, float timestamp);
    void recordDeath(cocos2d::CCPoint position, float percent);
    void finalizeAttempt(float deathPercent);

    std::vector<DeathPoint> const& getDeathPoints() const;
    std::vector<GhostAttempt> const& getGhostAttempts() const;
    GhostAttempt const& getCurrentRecording() const;

    EmotionState calculateEmotion() const;
    float getFrustrationLevel() const;
    float getProgressMomentum() const;
    int getDeathCountAt(cocos2d::CCPoint pos, float radius) const;

    int getTotalDeaths() const { return m_totalDeaths; }
    int getCurrentAttempt() const { return m_currentAttempt; }
    float getBestPercent() const { return m_bestPercent; }
    float getLastPercent() const { return m_lastPercent; }

    void clearForNewLevel();

private:
    DimensionManager() = default;

    std::vector<DeathPoint> m_deathPoints;
    std::vector<GhostAttempt> m_ghostAttempts;
    GhostAttempt m_currentRecording;

    int m_totalDeaths = 0;
    int m_currentAttempt = 0;
    float m_bestPercent = 0.f;
    float m_lastPercent = 0.f;
    float m_recentDeathRate = 0.f; // deaths per second recently

    std::vector<float> m_recentDeathTimes;
    std::vector<float> m_progressHistory;
};
