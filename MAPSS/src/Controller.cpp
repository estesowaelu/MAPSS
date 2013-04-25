//
//  Controller.cpp
//  MAPSS
//
//  Created by Robert Hodgin on 4/26/12.
//
//  Edited by Tim Honeywell in Spring 2013
//

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Gl.h"
#include "cinder/Rand.h"
#include "Controller.h"

using namespace ci;
using std::vector;

Controller::Controller() {}

Controller::Controller( Room *room, int maxLanterns ) {
	mRoom			= room;
	mMaxLanterns	= maxLanterns;	
}

void Controller::update() {
	// LANTERNS
	for( std::vector<Lantern>::iterator it = mLanterns.begin(); it != mLanterns.end(); ){
		if( it->mIsDead ){
			it = mLanterns.erase( it );
		} else {
			it->update( mRoom->mTimeAdjusted, mRoom->getFloorLevel() );
			++it;
		}
	}

	// SORT LANTERNS
	sort( mLanterns.begin(), mLanterns.end(), depthSortFunc );
}

void Controller::drawLanterns( gl::GlslProg *shader ) {
	for( std::vector<Lantern>::iterator it = mLanterns.begin(); it != mLanterns.end(); ++it ){
		shader->uniform( "color", it->mColor );
		it->draw();
	}
}

void Controller::drawLanternGlows( const Vec3f &right, const Vec3f &up ) {
	for( std::vector<Lantern>::iterator it = mLanterns.begin(); it != mLanterns.end(); ++it ){
		float radius = it->mRadius * 10.0f;// * it->mVisiblePer * 10.0f;
		gl::color( Color( it->mColor ) );
		gl::drawBillboard( it->mPos, Vec2f( radius, radius ), 0.0f, right, up );
		gl::color( Color::white() );
		gl::drawBillboard( it->mPos, Vec2f( radius, radius ) * 0.35f, 0.0f, right, up );
	}
}

void Controller::addLantern( const Vec3f &pos, Color col ) {
	if( mNumLanterns < mMaxLanterns ){
		mLanterns.push_back( Lantern( pos, col ) );
	}
}

bool depthSortFunc( Lantern a, Lantern b ) {
	return a.mPos.z > b.mPos.z;
}
