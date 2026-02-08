#pragma once
#include <vector>
#include <cmath>

enum class PFGravity { Normal, Flipped };

struct PFPlayer {
    double x = 0, y = 0;
    double yVel = 0;
    bool isDead = false;
    bool onGround = false;
    PFGravity gravity = PFGravity::Normal;
    
    double getSizeMult() const { return 1.0; }
};

struct PFObject {
    double x = 0, y = 0;
    double width = 30.0, height = 30.0;
    bool isHazard = false;
    bool isSolid = false;
    bool isPortal = false;
    bool isPad = false;
    bool isOrb = false;
    bool isTrigger = false;

    bool intersects(double px, double py, double pw, double ph) const {
        return std::abs(x - px) < (width + pw) / 2.0 &&
               std::abs(y - py) < (height + ph) / 2.0;
    }
};

struct LevelPhysicsData {};

class PhysicsEngine {
public:
    void initialize(const LevelPhysicsData& data, double tps) {}
    void reset() { m_player.isDead = false; }
    bool isDead() const { return m_player.isDead; }
    
    void checkCollisions();

private:
    PFPlayer m_player;
    bool m_prevHolding = false;

    std::vector<const PFObject*> getObjectsNear(double x, double range) { return {}; }
    void handlePortal(const PFObject& obj) {}
    void handlePad(const PFObject& obj) {}
    void handleOrb(const PFObject& obj, bool holding) {}
};
