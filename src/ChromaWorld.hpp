#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ChromaWorld : public cocos2d::CCNode {
public:
    static ChromaWorld* create();
    bool init() override;

    void update(float dt) override;
    void onMusicBeat(float bpm);

    void createVignette(cocos2d::CCLayer* parent);
    void createRhythmPulse(cocos2d::CCLayer* parent);
    void createDimensionFracture(cocos2d::CCPoint position, int intensity);

    void setCurrentBPM(float bpm) { m_bpm = bpm; }

private:
    float m_time = 0.f;
    float m_bpm = 120.f;
    float m_lastBeatTime = 0.f;
    float m_beatAccumulator = 0.f;
    float m_pulseScale = 1.f;
    float m_vignetteOpacity = 0.f;

    cocos2d::CCLayerColor* m_vignetteTop = nullptr;
    cocos2d::CCLayerColor* m_vignetteBottom = nullptr;
    cocos2d::CCLayerColor* m_vignetteLeft = nullptr;
    cocos2d::CCLayerColor* m_vignetteRight = nullptr;
    cocos2d::CCLayerColor* m_pulseOverlay = nullptr;

    std::vector<cocos2d::CCNode*> m_fractures;
};
