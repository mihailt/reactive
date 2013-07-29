#pragma once

#include "cinder/gl/gl.h"
#include "cinder/Vector.h"

class Particle{
public:
    
    Particle( const ci::Vec2f& position, float radius, float mass, float drag );
    
    void update();
    void draw();
    
    ci::Vec2f position, prevPosition, anchor, vel;
    ci::Vec2f forces;
    float radius;
    float mass;
    float drag;
};