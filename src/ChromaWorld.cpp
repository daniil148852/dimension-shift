#include "ChromaWorld.hpp"
#include "DimensionManager.hpp"
#include "EmotionEngine.hpp"
#include <cmath>

ChromaWorld* ChromaWorld::create() {
    auto ret = new ChromaWorld();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ChromaWorld::init() {
    if (!CCNode::init()) return false;
    this->scheduleUpdate();
    return true;
}

void ChromaWorld::update(float dt) {
    m_time += dt;

    // Rhythm pulse logic
    if (Mod::get()->getSettingValue<bool>("rhythm-pulse")) {
        float beatInterval = 60.f / m_bpm;
        m_beatAccumulator += dt;

        if (m_beatAccumulator >= beatInterval) {
            m_beatAccumulator -= beatInterval;
            onMusicBeat(m_bpm);
        }

        // Decay pulse
        m_pulseScale = 1.f + (m_pulseScale - 1.f) * std::pow(0.05f, dt);

        if (m_pulseOverlay) {
            float alpha = std::max(0.f, (m_pulseScale - 1.f) * 500.f);
            m_pulseOverlay->setOpacity(static_cast<GLubyte>(std::min(50.f, alpha)));
        }
    }

    // Update vignette based on emotion
    auto emotion = EmotionEngine::get();
    float targetVignette = emotion->getVignetteIntensity();
    m_vignetteOpacity += (targetVignette - m_vignetteOpacity) * dt * 2.f;

    if (m_vignetteTop) {
        GLubyte alpha = static_cast<GLubyte>(m_vignetteOpacity * 180.f);
        auto tint = emotion->getCurrentTint();
        m_vignetteTop->setColor(tint);
        m_vignetteTop->setOpacity(alpha);
        m_vignetteBottom->setColor(tint);
        m_vignetteBottom->setOpacity(alpha);
        m_vignetteLeft->setColor(tint);
        m_vignetteLeft->setOpacity(static_cast<GLubyte>(alpha * 0.7f));
        m_vignetteRight->setColor(tint);
        m_vignetteRight->setOpacity(static_cast<GLubyte>(alpha * 0.7f));
    }

    // Animate fractures
    for (auto fracture : m_fractures) {
        if (!fracture) continue;
        // Subtle pulsing glow
        float pulse = 0.6f + 0.4f * std::sin(m_time * 2.f + fracture->getPositionX() * 0.01f);
        
        // Cast to CCNodeRGBA to use setOpacity
        if (auto rgba = dynamic_cast<cocos2d::CCNodeRGBA*>(fracture)) {
            rgba->setOpacity(static_cast<GLubyte>(pulse * 200));
        }

        // Very slow rotation
        fracture->setRotation(fracture->getRotation() + dt * 5.f);
    }
}

void ChromaWorld::onMusicBeat(float bpm) {
    float intensity = Mod::get()->getSettingValue<double>("pulse-intensity");

    // Scale pulse based on BPM intensity
    m_pulseScale = 1.f + 0.02f * intensity;

    // Apply scale to parent if it's the game layer
    auto parent = this->getParent();
    if (parent) {
        // Subtle background scale pulse
        float bgScale = 1.f + 0.005f * intensity;
        parent->setScale(bgScale);

        // Schedule scale back
        parent->runAction(cocos2d::CCScaleTo::create(60.f / bpm * 0.5f, 1.f));
    }
}

void ChromaWorld::createVignette(cocos2d::CCLayer* parent) {
    if (!parent) return;

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    float vignetteThickness = 80.f;

    // Top vignette
    m_vignetteTop = cocos2d::CCLayerColor::create({0, 0, 0, 0},
        winSize.width, vignetteThickness);
    m_vignetteTop->setPosition({0, winSize.height - vignetteThickness});
    m_vignetteTop->setZOrder(999);
    parent->addChild(m_vignetteTop);

    // Bottom vignette
    m_vignetteBottom = cocos2d::CCLayerColor::create({0, 0, 0, 0},
        winSize.width, vignetteThickness);
    m_vignetteBottom->setPosition({0, 0});
    m_vignetteBottom->setZOrder(999);
    parent->addChild(m_vignetteBottom);

    // Left vignette
    m_vignetteLeft = cocos2d::CCLayerColor::create({0, 0, 0, 0},
        vignetteThickness, winSize.height);
    m_vignetteLeft->setPosition({0, 0});
    m_vignetteLeft->setZOrder(999);
    parent->addChild(m_vignetteLeft);

    // Right vignette
    m_vignetteRight = cocos2d::CCLayerColor::create({0, 0, 0, 0},
        vignetteThickness, winSize.height);
    m_vignetteRight->setPosition({winSize.width - vignetteThickness, 0});
    m_vignetteRight->setZOrder(999);
    parent->addChild(m_vignetteRight);
}

void ChromaWorld::createRhythmPulse(cocos2d::CCLayer* parent) {
    if (!parent) return;

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    m_pulseOverlay = cocos2d::CCLayerColor::create({255, 255, 255, 0},
        winSize.width, winSize.height);
    m_pulseOverlay->setZOrder(998);
    m_pulseOverlay->setBlendFunc({GL_SRC_ALPHA, GL_ONE}); // Additive
    parent->addChild(m_pulseOverlay);
}

void ChromaWorld::createDimensionFracture(cocos2d::CCPoint position, int intensity) {
    if (!Mod::get()->getSettingValue<bool>("dimension-fracture")) return;

    // Create a "crack" visual at death position using CCNodeRGBA for opacity support
    auto fractureNode = cocos2d::CCNodeRGBA::create();
    fractureNode->setPosition(position);
    fractureNode->setCascadeOpacityEnabled(true);

    // Create crack lines using draw node
    auto drawNode = cocos2d::CCDrawNode::create();

    // Generate crack pattern
    float crackSize = 20.f + intensity * 8.f;
    int numCracks = 3 + std::min(intensity, 8);

    auto emotionColor = EmotionEngine::get()->getCurrentTint();
    cocos2d::ccColor4F crackColor = {
        emotionColor.r / 255.f,
        emotionColor.g / 255.f,
        emotionColor.b / 255.f,
        0.8f
    };

    for (int i = 0; i < numCracks; i++) {
        float angle = (static_cast<float>(i) / numCracks) * 360.f + (rand() % 30 - 15);
        float rad = angle * M_PI / 180.f;
        float length = crackSize * (0.5f + static_cast<float>(rand() % 50) / 100.f);

        cocos2d::CCPoint start = {0, 0};
        cocos2d::CCPoint end = {
            std::cos(rad) * length,
            std::sin(rad) * length
        };

        // Main crack line
        drawNode->drawSegment(start, end, 1.5f, crackColor);

        // Sub-cracks (branches)
        if (intensity > 3) {
            float branchAngle = rad + (rand() % 60 - 30) * M_PI / 180.f;
            float branchLen = length * 0.4f;
            cocos2d::CCPoint mid = {
                std::cos(rad) * length * 0.6f,
                std::sin(rad) * length * 0.6f
            };
            cocos2d::CCPoint branchEnd = {
                mid.x + std::cos(branchAngle) * branchLen,
                mid.y + std::sin(branchAngle) * branchLen
            };
            drawNode->drawSegment(mid, branchEnd, 1.f, {crackColor.r, crackColor.g, crackColor.b, 0.5f});
        }
    }

    // Central glow dot
    drawNode->drawDot({0, 0}, 3.f + intensity * 0.5f, {crackColor.r, crackColor.g, crackColor.b, 0.6f});

    fractureNode->addChild(drawNode);

    // Add a glow sprite behind the cracks
    auto glow = cocos2d::CCSprite::create("GJ_square01.png");
    if (glow) {
        glow->setColor(emotionColor);
        glow->setOpacity(30 + intensity * 5);
        glow->setScale(crackSize / 50.f);
        glow->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        fractureNode->addChild(glow, -1);
    }

    fractureNode->setZOrder(5);
    m_fractures.push_back(fractureNode);

    // Add to parent
    if (this->getParent()) {
        this->getParent()->addChild(fractureNode);
    }
}
