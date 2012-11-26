//
//  ParticleController.h
//  matrixP
//
//  Created by Tim Honeywell on 12/11/04.
//
//

#pragma once

#include "Particle.h"
#include <list>

class ParticleController {
public:
	ParticleController();
	ParticleController(int res);
	void update(int color, bool ordered, const ci::Vec2i &mouseLoc);
	void draw();
	void addParticle(int xi, int yi, int res);
	void addParticles(int amt, int res);
	void removeParticles(int amt);
	
	std::list<Particle>	mParticles;
	
	int mXRes;
	int mYRes;
};
