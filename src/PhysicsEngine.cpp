#include "PhysicsEngine.hpp"

void PhysicsEngine::checkCollisions() {
    // FIX: Make hitbox smaller (forgiveness factor)
    // GD hitboxes are actually quite small inside the sprites
    double hitboxScale = 0.5; 
    
    double playerW = 30.0 * m_player.getSizeMult() * hitboxScale;
    double playerH = 30.0 * m_player.getSizeMult() * hitboxScale;

    // Search range needs to be larger to find nearby objects
    auto nearbyObjects = getObjectsNear(m_player.x, 100.0);

    for (const auto* obj : nearbyObjects) {
        // For logic checks (portals/pads), use slightly larger hitbox
        double triggerScale = 0.8;
        if (obj->isPortal || obj->isPad || obj->isOrb || obj->isTrigger) {
             if (obj->intersects(m_player.x, m_player.y, 30.0 * triggerScale, 30.0 * triggerScale)) {
                 if (obj->isPortal) handlePortal(*obj);
                 else if (obj->isPad) handlePad(*obj);
                 else if (obj->isOrb) handleOrb(*obj, m_prevHolding);
             }
             continue; // Triggers don't kill or block
        }

        // Hazard/Solid collision
        if (!obj->intersects(m_player.x, m_player.y, playerW, playerH)) {
            continue;
        }

        if (obj->isHazard) {
            m_player.isDead = true;
            return;
        }

        if (obj->isSolid) {
            double dx = m_player.x - obj->x;
            double dy = m_player.y - obj->y;
            double combinedH = (obj->height * hitboxScale + playerH) / 2.0;
            double combinedW = (obj->width * hitboxScale + playerW) / 2.0;

            // Vertical landing
            if (std::abs(dx) < combinedW * 0.8) { // Only land if we are mostly above/below
                if (dy > 0 && m_player.gravity == PFGravity::Normal) {
                    if (m_player.yVel <= 0) { // Only land if falling or flat
                        m_player.y = obj->y + obj->height/2.0 + (30.0 * m_player.getSizeMult())/2.0 - 1.0; 
                        m_player.yVel = 0;
                        m_player.onGround = true;
                    }
                } else if (dy < 0 && m_player.gravity == PFGravity::Flipped) {
                    if (m_player.yVel >= 0) {
                        m_player.y = obj->y - obj->height/2.0 - (30.0 * m_player.getSizeMult())/2.0 + 1.0;
                        m_player.yVel = 0;
                        m_player.onGround = true;
                    }
                } else {
                    // Hit ceiling/floor while moving opposite way -> death or stop
                    m_player.isDead = true; 
                    return;
                }
            } else {
                // Horizontal crash
                m_player.isDead = true;
                return;
            }
        }
    }
}
