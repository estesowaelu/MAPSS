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

Controller::Controller(){}

Controller::Controller( Room *room, int maxLanterns )
{
	mRoom			= room;
	mMaxLanterns	= maxLanterns;
	
//	mNumPredators	= predatorFboDim * predatorFboDim;
//	for( int i=0; i<mNumPredators; i++ ){
//		mPredators.push_back( Predator( Vec3f::zero() ) );
//	}
}

void Controller::updatePredatorBodies( gl::Fbo *fbo )
{
	// BOTH THESE METHODS ARE TOO SLOW.
	// IS THERE NO WAY TO READ OUT THE CONTENTS OF A TINY FBO TEXTURE
	// WITHOUT THE FRAMERATE DROPPING FROM 60 TO 30?
	
//	gl::disableDepthRead();
//	gl::disableDepthWrite();
//	gl::disableAlphaBlending();
//	
//	int index = 0;
//	Surface32f predatorSurface( fbo->getTexture() );
//	Surface32f::Iter it = predatorSurface.getIter();
//	while( it.line() ){
//		while( it.pixel() ){
//			mPredators[index].update( Vec3f( it.r(), it.g(), it.b() ) );
//			index ++;
//		}
//	}
	
//	int index = 0;
//	GLfloat pixel[4];
//	int fboDim = fbo->getWidth();
//	fbo->bindFramebuffer();
//	for( int y=0; y<fboDim; y++ ){
//		for( int x=0; x<fboDim; x++ ){
//			glReadPixels( x, y, 1, 1, GL_RGB, GL_FLOAT, (void *)pixel );
//			mPredators[index].update( Vec3f( pixel[0], pixel[1], pixel[2] ) );
//			index ++;
//		}
//	}
//	fbo->unbindFramebuffer();
}

void Controller::update()
{
	// LANTERNS
	for( std::vector<Lantern>::iterator it = mLanterns.begin(); it != mLanterns.end(); ){
		if( it->mIsDead ){
			it = mLanterns.erase( it );
		} else {
			it->update( mRoom->mTimeAdjusted, mRoom->getFloorLevel() );
			++it;
		}
	}
	
	// ADD LANTERN IF REQUIRED
	mNumLanterns = mLanterns.size();
	if( mRoom->isPowerOn() ){
		if( ( Rand::randFloat() < 0.00175f && mNumLanterns < 3 ) || mNumLanterns < 1 ){
			addLantern( mRoom->getRandCeilingPos() );
		}
	}
	
	
	// SORT LANTERNS
	sort( mLanterns.begin(), mLanterns.end(), depthSortFunc );
}



void Controller::drawLanterns( gl::GlslProg *shader )
{
	for( std::vector<Lantern>::iterator it = mLanterns.begin(); it != mLanterns.end(); ++it ){
		shader->uniform( "color", it->mColor );
		it->draw();
	}
}

void Controller::drawLanternGlows( const Vec3f &right, const Vec3f &up )
{
	for( std::vector<Lantern>::iterator it = mLanterns.begin(); it != mLanterns.end(); ++it ){
		float radius = it->mRadius * 10.0f;// * it->mVisiblePer * 10.0f;
		gl::color( Color( it->mColor ) );
		gl::drawBillboard( it->mPos, Vec2f( radius, radius ), 0.0f, right, up );
		gl::color( Color::white() );
		gl::drawBillboard( it->mPos, Vec2f( radius, radius ) * 0.35f, 0.0f, right, up );
	}
}


void Controller::addLantern( const Vec3f &pos )
{
	if( mNumLanterns < mMaxLanterns ){
		mLanterns.push_back( Lantern( pos ) );
	}
}


bool depthSortFunc( Lantern a, Lantern b ){
	return a.mPos.z > b.mPos.z;
}