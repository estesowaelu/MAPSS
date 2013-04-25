//
//  Controller.h
//  MAPSS
//
//  Created by Robert Hodgin on 4/26/12.
//
//  Edited by Tim Honeywell in Spring 2013
//

#pragma once
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "Room.h"
#include "Lantern.h"
#include <vector>

class Controller {
public:
	Controller();
	Controller( Room *room, int maxLanterns );
	void update();
	void drawLanterns( ci::gl::GlslProg *shader );
	void drawLanternGlows( const ci::Vec3f &right, const ci::Vec3f &up );
	void addLantern( const ci::Vec3f &pos );
	Room					*mRoom;

	int						mNumLanterns;
	int						mMaxLanterns;
	std::vector<Lantern>	mLanterns;
};

bool depthSortFunc( Lantern a, Lantern b );


