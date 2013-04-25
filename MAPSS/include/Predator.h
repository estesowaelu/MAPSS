//
//  Predator.h
//  MAPSS
//
//  Created by Robert Hodgin on 5/12/12.
//
//  Edited by Tim Honeywell in Spring 2013
//

#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include <vector>

class Predator {
public:
	Predator();
	Predator( const ci::Vec3f &v );
	void update( const ci::Vec3f &v );
	void draw();
	
	int mLen;
	ci::Vec3f	mPos;
	std::vector<ci::Vec3f>	mPositions;
	
	float					mRadius;
	std::vector<float>		mRadii;
};