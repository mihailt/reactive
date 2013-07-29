#pragma once

#include "Particle.h"
#include <vector>

class ParticleSystem{
public:
    
    ~ParticleSystem();
    
    void update();
    void draw();
    
    void addParticle( Particle *particle );
    void destroyParticle( Particle *particle );
    
    std::vector<Particle*> particles;
    
};