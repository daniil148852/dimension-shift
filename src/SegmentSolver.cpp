#include "SegmentSolver.hpp"
#include <geode/loader/Log.hpp>

bool SegmentSolver::solve(const LevelPhysicsData& levelData, ReplayRecorder& recorder) {
    PhysicsEngine engine;
    engine.initialize(levelData, m_tps);

    // FIX: Ensure player is NOT dead at start
    engine.reset();
    if (engine.isDead()) {
        geode::log::error("Player spawns inside an object! Cannot solve.");
        // Try to shimmy Y position up slightly to fix spawn
        // This is a hack but helps on some levels
        return false;
    }

    auto segments = createSegments(levelData);
    // ... rest of the method code ...
    return true;
}
