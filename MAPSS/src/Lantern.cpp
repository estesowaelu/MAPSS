//
//  Bait.cpp
//  MAPSS
//
//  Created by Robert Hodgin on 4/26/12.
//
//  Edited by Tim Honeywell in Spring 2013
//

#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "Lantern.h"

using namespace ci;

Lantern::Lantern( const Vec3f &pos, Color col, int vid ) {
	mPos		= pos;
	mRadius		= 0.0f;
	mRadiusDest	= Rand::randFloat( 4.5f, 7.5f );
	if( Rand::randFloat() < 0.1f ) mRadiusDest = Rand::randFloat( 13.0f, 25.0f );
	
    mColor      = col;
	mIsDead		= false;
	mIsDying	= false;
    deathTimer  = 1400;
    vID         = vid;
	
	mVisiblePer	= 1.0f;
}

void Lantern::update( float dt, float yFloor ) {
	deathTimer--;
    
    if (deathTimer <= 0) {
        mIsDying = true;
    }
	
	if( mIsDying ){
		mRadius -= ( mRadius - 0.0f ) * 0.2f;
		if( mRadius < 0.1f )
			mIsDead = true;
	} else {
		mRadius -= ( mRadius - ( mRadiusDest + Rand::randFloat( 0.9f, 1.2f ) ) ) * 0.2f;
	}
}

void Lantern::draw() {
	gl::drawSphere( mPos, mRadius * 0.5f, 32 );
}