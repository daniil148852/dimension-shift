#include "DimensionManager.hpp"

DimensionManager* DimensionManager::get() {
    static DimensionManager instance;
    return &instance;
}

void DimensionManager::reset() {
    m_currentRecording.frames.clear();
    m_currentRecording.attemptNumber = m_currentAttempt;
}

void DimensionManager::clearForNewLevel() {
    m_deathPoints.clear();
    m_ghostAttempts.clear();
    m_currentRecording.frames.clear();
    m_totalDeaths = 0;
    m_currentAttempt = 0;
    m_bestPercent = 0.f;
    m_lastPercent = 0.f;
    m_recentDeathTimes.clear();
    m_progressHistory.clear();
}

void DimensionManager::recordFrame(PlayerObject* player, float timestamp) {
    if (!player) return;

    GhostFrame frame;
    frame.position = player->getPosition();
    frame.rotation = player->getRotation();
    frame.timestamp = timestamp;
    frame.isFlipped = player->m_isUpsideDown;
    frame.isCube = !player->m_isShip && !player->m_isBall && !player->m_isBird &&
                   !player->m_isDart && !player->m_isRobot && !player->m_isSpider &&
                   !player->m_isSwing;
    frame.isShip = player->m_isShip;
    frame.isBall = player->m_isBall;
    frame.isUfo = player->m_isBird;
    frame.isWave = player->m_isDart;
    frame.isRobot = player->m_isRobot;
    frame.isSpider = player->m_isSpider;
    frame.isDart = player->m_isSwing;
    frame.yVelocity = player->m_yVelocity;

    m_currentRecording.frames.push_back(frame);
}

void DimensionManager::recordDeath(cocos2d::CCPoint position, float percent) {
    m_totalDeaths++;

    // Check if we already have a death near this position
    bool found = false;
    for (auto& dp : m_deathPoints) {
        if (dp.position.getDistance(position) < 50.f) {
            dp.deathCount++;
            found = true;
            break;
        }
    }

    if (!found) {
        m_deathPoints.push_back({position, 1, percent});
    }

    // Track recent death timing for frustration calculation
    auto now = static_cast<float>(clock()) / CLOCKS_PER_SEC;
    m_recentDeathTimes.push_back(now);

    // Only keep last 30 seconds of death times
    while (!m_recentDeathTimes.empty() && (now - m_recentDeathTimes.front()) > 30.f) {
        m_recentDeathTimes.erase(m_recentDeathTimes.begin());
    }
}

void DimensionManager::finalizeAttempt(float deathPercent) {
    m_currentRecording.deathPercent = deathPercent;
    m_lastPercent = deathPercent;

    if (deathPercent > m_bestPercent) {
        m_bestPercent = deathPercent;
    }

    m_progressHistory.push_back(deathPercent);

    // Store ghost attempt (keep limited number)
    int maxGhosts = Mod::get()->getSettingValue<int64_t>("max-ghosts");
    if (static_cast<int>(m_ghostAttempts.size()) >= maxGhosts) {
        // Remove the oldest one, but prefer keeping diverse death points
        m_ghostAttempts.erase(m_ghostAttempts.begin());
    }

    if (!m_currentRecording.frames.empty()) {
        m_ghostAttempts.push_back(m_currentRecording);
    }

    m_currentAttempt++;
    reset();
}

std::vector<DeathPoint> const& DimensionManager::getDeathPoints() const {
    return m_deathPoints;
}

std::vector<GhostAttempt> const& DimensionManager::getGhostAttempts() const {
    return m_ghostAttempts;
}

GhostAttempt const& DimensionManager::getCurrentRecording() const {
    return m_currentRecording;
}

EmotionState DimensionManager::calculateEmotion() const {
    float frustration = getFrustrationLevel();
    float momentum = getProgressMomentum();

    if (m_bestPercent > 95.f && m_totalDeaths > 0) {
        return EmotionState::Triumph;
    }

    if (frustration > 0.8f) {
        return EmotionState::Frustration;
    }

    if (momentum > 0.5f && frustration < 0.3f) {
        return EmotionState::Hope;
    }

    if (frustration > 0.4f && momentum > 0.2f) {
        return EmotionState::Determination;
    }

    if (m_totalDeaths == 0 && m_currentAttempt < 3) {
        return EmotionState::Neutral;
    }

    if (frustration < 0.15f && m_currentAttempt > 5) {
        return EmotionState::Zen;
    }

    return EmotionState::Neutral;
}

float DimensionManager::getFrustrationLevel() const {
    if (m_recentDeathTimes.size() < 2) return 0.f;

    auto now = static_cast<float>(clock()) / CLOCKS_PER_SEC;
    float recentWindow = 15.f;
    int recentDeaths = 0;

    for (auto& t : m_recentDeathTimes) {
        if ((now - t) < recentWindow) recentDeaths++;
    }

    // Also consider if progress is stagnating
    float stagnation = 0.f;
    if (m_progressHistory.size() >= 5) {
        float recent5Avg = 0.f;
        for (int i = static_cast<int>(m_progressHistory.size()) - 5;
             i < static_cast<int>(m_progressHistory.size()); i++) {
            recent5Avg += m_progressHistory[i];
        }
        recent5Avg /= 5.f;

        float variance = 0.f;
        for (int i = static_cast<int>(m_progressHistory.size()) - 5;
             i < static_cast<int>(m_progressHistory.size()); i++) {
            float diff = m_progressHistory[i] - recent5Avg;
            variance += diff * diff;
        }
        variance /= 5.f;

        // Low variance + not near 100% = stagnation
        if (variance < 25.f && recent5Avg < 80.f) {
            stagnation = 1.f - (variance / 25.f);
        }
    }

    float deathRate = static_cast<float>(recentDeaths) / recentWindow;
    float frustration = std::min(1.f, (deathRate * 0.6f) + (stagnation * 0.4f));
    return frustration;
}

float DimensionManager::getProgressMomentum() const {
    if (m_progressHistory.size() < 3) return 0.f;

    // Compare last 3 attempts average vs previous 3
    int sz = static_cast<int>(m_progressHistory.size());
    if (sz < 6) {
        // Just check if trending up
        float last = m_progressHistory.back();
        float prev = m_progressHistory[sz - 2];
        return std::max(0.f, std::min(1.f, (last - prev) / 20.f + 0.5f));
    }

    float recent3 = 0.f, prev3 = 0.f;
    for (int i = sz - 3; i < sz; i++) recent3 += m_progressHistory[i];
    for (int i = sz - 6; i < sz - 3; i++) prev3 += m_progressHistory[i];
    recent3 /= 3.f;
    prev3 /= 3.f;

    float improvement = (recent3 - prev3) / std::max(1.f, prev3);
    return std::max(0.f, std::min(1.f, improvement + 0.5f));
}

int DimensionManager::getDeathCountAt(cocos2d::CCPoint pos, float radius) const {
    int count = 0;
    for (auto& dp : m_deathPoints) {
        if (dp.position.getDistance(pos) < radius) {
            count += dp.deathCount;
        }
    }
    return count;
}
