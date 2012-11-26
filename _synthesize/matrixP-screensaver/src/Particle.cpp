//
//  Particle.cpp
//  matrixP
//
//  Created by Tim Honeywell on 12/11/04.
//
//

#include "Particle.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/app/AppBasic.h"
#include "cinder/CinderMath.h"

using namespace ci;

Particle::Particle() {
}

Particle::Particle(Vec2f loc, int res) {
	mLoc	= loc;
	mDir	= Rand::randVec2f();
	mDirToCursor	= Vec2f::zero();
	mVel			= 0.0f;
	mRadius			= 0.0f;
	mScale			= 2.0f;
	cMod = 0.0f;
	rMod = 0.0f;
	gMod = 0.0f;
	bMod = 0.0f;
	mColor = Color(1.0f, 1.0f, 1.0f);
	resolution = res;
}

void Particle::update(int color, bool ordered, const Vec2i &mouseLoc) {
	// calculate morph values
	mDirToCursor		= mouseLoc - mLoc;
	float distToCursor	= mDirToCursor.length();
	mDirToCursor.normalize();
	float mDTCNxC = pow(mDirToCursor.x, 3.0f);
	float mDTCNyC = pow(mDirToCursor.y, 3.0f);
	float hypotDCN = hypotf(mDTCNxC, mDTCNyC);

	// update color if needed
	updateColor(color, hypotDCN);

	// update order if needed
	if(ordered) updateSize(distToCursor, hypotDCN);
	else {
		mRadius += Rand::randFloat(-0.5f, 0.5f);
		boundFloat(mRadius, false);		
	}
}

void Particle::updateColor(int color, float tint) {
	switch (color) {
		case 1:
			// 1=greyscale
			cMod += Rand::randFloat(-0.1f, 0.1f);
			boundFloat(cMod, true);
			mColor = Color(cMod, cMod, cMod);
			break;
		case 2:
			// 2=random color
			rMod += Rand::randFloat(-0.1f, 0.1f);
			gMod += Rand::randFloat(-0.1f, 0.1f);
			bMod += Rand::randFloat(-0.1f, 0.1f);
			boundFloat(rMod, true);
			boundFloat(gMod, true);
			boundFloat(bMod, true);
			mColor = Color(rMod, gMod, bMod);
			break;
		case 3:
			// 3=gradient
			mColor = Color(tint, 0.0f, 0.0f);
			break;
		default:
			break;
	}	
}

void Particle::updateSize(float distToCursor, float hypotDCN) {
		float time			= app::getElapsedSeconds();
		float dist			= distToCursor * 0.025f;
		float offset		= cos(((dist - time)*2)/3) + 1.0f;

		mDirToCursor		*= offset * 100.0f;
		
		Vec2f newLoc		= mLoc + mDirToCursor;
		
		newLoc.x = constrain(newLoc.x, 0.0f, app::getWindowWidth() - 1.0f);
		newLoc.y = constrain(newLoc.y, 0.0f, app::getWindowHeight() - 1.0f);
		
		mRadius				= hypotDCN * resolution * 0.25f;
}

void Particle::draw() {
	gl::color(mColor);
	gl::drawSolidCircle(mLoc + mDirToCursor * 0.2f, mRadius);
}

void Particle::boundFloat(float& i, bool colored) {
	if(i <= 0.0f) i = 0.0f;
	else if(colored && i >= 1.0f) i = 1.0f;
	else if(!colored && i >= 5.0f) i = 5.0f;
}
