//
//  Particle.h
//  matrixP
//
//  Created by Tim Honeywell on 12/11/04.
//
//

#pragma once

#include "cinder/Vector.h"
#include "cinder/Color.h"
#include <vector>

class Particle {
public:
	Particle();
	Particle(ci::Vec2f, int res);

	void update(int color, bool ordered, const ci::Vec2i &mouseLoc);
	void updateColor(int color, float tint);
	void updateSize(float distToCursor, float hypotDCN);
	void draw();

	void boundFloat(float& i, bool colored);
	
	ci::Vec2f	mLoc;
	ci::Vec2f	mLocPer;
	ci::Vec2f	mDir;
	ci::Vec2f	mDirToCursor;
	float		mVel;
	
	float		mRadius;
	float		mScale;

	float		cMod;
	float		rMod;
	float		gMod;
	float		bMod;
	
	int			resolution;

	ci::Color	mColor;

};
