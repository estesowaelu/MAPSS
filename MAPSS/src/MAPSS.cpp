//
//  Main.cpp
//  MAPSS
//
//  Created by Robert Hodgin
//
//  Edited by Tim Honeywell in Spring 2013
//

#include <vector>
#include <boost/thread.hpp>

#include "Leap.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "cinder/params/Params.h"

#include "SpringCam.h"
#include "Room.h"
#include "Controller.h"
#include "Lantern.h"
#include "Resources.h"

using namespace std;
using namespace ci;
using namespace ci::app;

#define APP_WIDTH		1440
#define APP_HEIGHT		900
#define ROOM_WIDTH		350
#define ROOM_HEIGHT		200
#define ROOM_DEPTH		150
#define ROOM_FBO_RES	2
#define FBO_DIM			50//167
#define P_FBO_DIM		5
#define MAX_LANTERNS	24

class MAPSSListener : public Leap::Listener {
public:
	virtual void onInit(const Leap::Controller& leap) {
		leap.enableGesture( Leap::Gesture::TYPE_SCREEN_TAP );
		leap.enableGesture( Leap::Gesture::TYPE_KEY_TAP );
		leap.enableGesture( Leap::Gesture::TYPE_CIRCLE );
	}
};

class MAPSS : public AppBasic {
public:
	virtual void		prepareSettings( Settings *settings );
	virtual void		setup();
	void				adjustFboDim( int offset );
	void				initialize();
	void				setFboPositions( gl::Fbo fbo );
	void				setFboVelocities( gl::Fbo fbo );
	void				initVbo();
	virtual void		mouseDown( MouseEvent event );
	virtual void		mouseUp( MouseEvent event );
	virtual void		mouseMove( MouseEvent event );
	virtual void		mouseDrag( MouseEvent event );
	virtual void		mouseWheel( MouseEvent event );
	virtual void		keyDown( KeyEvent event );
	virtual void		update();
	void				drawIntoRoomFbo();
	void				drawInfoPanel();
	void				drawIntoVelocityFbo();
	void				drawIntoPositionFbo();
	void				drawIntoLanternsFbo();
	virtual void		draw();
	
	void processGestures();
    bool hasDroppedRecently;
    int dropTimer;

	Vec3f normalizeCoords(const Leap::Vector& vec);
	
	// CAMERA
	SpringCam			mSpringCam;
	
	// TEXTURES
	gl::Texture			mLanternGlowTex;
	gl::Texture			mIconTex;
	
	// SHADERS
	gl::GlslProg		mVelocityShader;
	gl::GlslProg		mPositionShader;
	gl::GlslProg		mP_VelocityShader;
	gl::GlslProg		mP_PositionShader;
	gl::GlslProg		mLanternShader;
	gl::GlslProg		mRoomShader;
	gl::GlslProg		mShader;
	gl::GlslProg		mP_Shader;
	
	// CONTROLLER
	Controller			mController;

	// LANTERNS (point lights)
	gl::Fbo				mLanternsFbo;
	
	// ROOM
	Room				mRoom;
	gl::Fbo				mRoomFbo;
	
	// VBOS
	gl::VboMesh			mVboMesh;
	gl::VboMesh			mP_VboMesh;
	
	// POSITION/VELOCITY FBOS
	gl::Fbo::Format		mRgba16Format;
	int					mFboDim;
	ci::Vec2f			mFboSize;
	ci::Area			mFboBounds;
	gl::Fbo				mPositionFbos[2];
	gl::Fbo				mVelocityFbos[2];
	int					mP_FboDim;
	ci::Vec2f			mP_FboSize;
	ci::Area			mP_FboBounds;
	gl::Fbo				mP_PositionFbos[2];
	gl::Fbo				mP_VelocityFbos[2];
	int					mThisFbo, mPrevFbo;
	
	// MOUSE
	Vec2f				mMousePos, mMouseDownPos, mMouseOffset;
	
	bool				mInitUpdateCalled;
	
private:
	bool mMousePressed;
	ci::Vec2i lastMousePos;
	Leap::Controller controller;
	MAPSSListener listener;
	Leap::Frame lastFrame;
};

//Handle Leap Gesture processing.
void MAPSS::processGestures() {
    if (dropTimer > 0) {
        dropTimer--;
        return;
    }
    
	Leap::Frame frame = controller.frame();
	if ( lastFrame == frame ) {
		return;
	}

	Leap::GestureList gestures =  lastFrame.isValid()       ?
	frame.gestures(lastFrame) :
	frame.gestures();
	lastFrame = frame;
	
	size_t numGestures = gestures.count();
	
	for (size_t i=0; i < numGestures; i++) {
		if (gestures[i].type() == Leap::Gesture::TYPE_SCREEN_TAP) {
            /////////// screen-taps should place yellow cubes //////////////
			Leap::ScreenTapGesture tap = gestures[i];
			ci::Vec3f eLoc = normalizeCoords(tap.position());
            console() << tap.position() << "\n"; console() << eLoc << "\n";
            Color lcolor = Color( 255, 255, 0 );
            dropTimer = 90;
			mController.addLantern(eLoc, lcolor, 0);
		} else if (gestures[i].type() == Leap::Gesture::TYPE_KEY_TAP) {
            /////////// key-taps should place red pyramids /////////////
			Leap::KeyTapGesture tap = gestures[i];
			ci::Vec3f eLoc = normalizeCoords(tap.position());
            Color lcolor = Color( 255, 0, 0 );
            dropTimer = 90;
			mController.addLantern(eLoc, lcolor, -1);
		} else if (gestures[i].type() == Leap::Gesture::TYPE_CIRCLE) {
            ///////// Circles should place blue spheres ///////////////
			Leap::CircleGesture circle = gestures[i];
			float progress = circle.progress();
			if (progress >= 1.0f) {
				ci::Vec3f eLoc = normalizeCoords(circle.center());
                Color lcolor = Color( 0, 0, 255 );
                dropTimer = 90;
                mController.addLantern(eLoc, lcolor, 1);
			}
		}
    }
}

Vec3f MAPSS::normalizeCoords(const Leap::Vector& vec) {
	ci::Vec3f result;
    result.x = ((-vec.x/10) * 7) * 2;
    result.y = (7/4 * vec.y - 187.5) * 2;
    result.z = ((-vec.z/4) * 3) * 2;
	return result;
}

void MAPSS::prepareSettings( Settings *settings ) {
	settings->setWindowSize( APP_WIDTH, APP_HEIGHT );
	settings->setFrameRate(120);
	settings->setFullScreen( true );
}

void MAPSS::setup() {
	// CAMERA	
	mSpringCam			= SpringCam( -420.0f, getWindowAspectRatio() );
	
	// POSITION/VELOCITY FBOS
	mRgba16Format.setColorInternalFormat( GL_RGBA16F_ARB );
	mRgba16Format.setMinFilter( GL_NEAREST );
	mRgba16Format.setMagFilter( GL_NEAREST );
	mThisFbo			= 0;
	mPrevFbo			= 1;
	
	// LANTERNS
	mLanternsFbo		= gl::Fbo( MAX_LANTERNS, 2, mRgba16Format );
	
	// TEXTURE FORMAT
	gl::Texture::Format mipFmt;
    mipFmt.enableMipmapping( true );
    mipFmt.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );    
    mipFmt.setMagFilter( GL_LINEAR );
	
	// TEXTURES
	mLanternGlowTex		= gl::Texture( loadImage( loadResource( RES_LANTERNGLOW_PNG ) ) );
	
	// LOAD SHADERS
	try {
		mVelocityShader		= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_VELOCITY_FRAG ) );
		mPositionShader		= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_POSITION_FRAG ) );
		mP_VelocityShader	= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_P_VELOCITY_FRAG ) );
		mP_PositionShader	= gl::GlslProg( loadResource( RES_PASSTHRU_VERT ),	loadResource( RES_P_POSITION_FRAG ) );
		mLanternShader		= gl::GlslProg( loadResource( RES_LANTERN_VERT ),	loadResource( RES_LANTERN_FRAG ) );
		mRoomShader			= gl::GlslProg( loadResource( RES_ROOM_VERT ),		loadResource( RES_ROOM_FRAG ) );
		mShader				= gl::GlslProg( loadResource( RES_VBOPOS_VERT ),	loadResource( RES_VBOPOS_FRAG ) );
		mP_Shader			= gl::GlslProg( loadResource( RES_P_VBOPOS_VERT ),	loadResource( RES_P_VBOPOS_FRAG ) );
	} catch( gl::GlslProgCompileExc e ) {
		std::cout << e.what() << std::endl;
		quit();
	}
	
	// ROOM
	gl::Fbo::Format roomFormat;
	roomFormat.setColorInternalFormat( GL_RGB );
	mRoomFbo			= gl::Fbo( APP_WIDTH/ROOM_FBO_RES, APP_HEIGHT/ROOM_FBO_RES, roomFormat );
	bool isPowerOn		= false;
	mRoom				= Room( Vec3f( ROOM_WIDTH, ROOM_HEIGHT, ROOM_DEPTH ), isPowerOn );
	mRoom.init();
	
	// CONTROLLER
	mController			= Controller( &mRoom, MAX_LANTERNS );
	
	// MOUSE
	mMousePos			= Vec2f::zero();
	mMouseDownPos		= Vec2f::zero();
	mMouseOffset		= Vec2f::zero();
	mMousePressed		= false;
		
	mInitUpdateCalled	= false;
    
    dropTimer = 0;
	
	gl::disableVerticalSync();
	
	lastMousePos.set(0, 0);
	controller.addListener( listener );

	initialize();
}

void MAPSS::initialize() {
	gl::disableAlphaBlending();
	gl::disableDepthWrite();
	gl::disableDepthRead();
	
	mFboDim				= FBO_DIM;
	mFboSize			= Vec2f( mFboDim, mFboDim );
	mFboBounds			= Area( 0, 0, mFboDim, mFboDim );
	mPositionFbos[0]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	mPositionFbos[1]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	mVelocityFbos[0]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	mVelocityFbos[1]	= gl::Fbo( mFboDim, mFboDim, mRgba16Format );
	
	setFboPositions( mPositionFbos[0] );
	setFboPositions( mPositionFbos[1] );
	setFboVelocities( mVelocityFbos[0] );
	setFboVelocities( mVelocityFbos[1] );
	
	mP_FboDim			= P_FBO_DIM;
	mP_FboSize			= Vec2f( mP_FboDim, mP_FboDim );
	mP_FboBounds		= Area( 0, 0, mP_FboDim, mP_FboDim );
	mP_PositionFbos[0]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	mP_PositionFbos[1]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	mP_VelocityFbos[0]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	mP_VelocityFbos[1]	= gl::Fbo( mP_FboDim, mP_FboDim, mRgba16Format );
	
	initVbo();
}

void MAPSS::setFboPositions( gl::Fbo fbo ) {
	// FISH POSITION
	Surface32f posSurface( fbo.getTexture() );
	Surface32f::Iter it = posSurface.getIter();
	while( it.line() ){
		float y = (float)it.y()/(float)it.getHeight() - 0.5f;
		while( it.pixel() ){
			float per		= (float)it.x()/(float)it.getWidth();
			float angle		= per * M_PI * 2.0f;
			float radius	= 100.0f;
			float cosA		= cos( angle );
			float sinA		= sin( angle );
			Vec3f p			= Vec3f( cosA, y, sinA ) * radius;
			
			it.r() = p.x;
			it.g() = p.y;
			it.b() = p.z;
			it.a() = Rand::randFloat( 0.7f, 1.0f );
		}
	}
	
	gl::Texture posTexture( posSurface );
	fbo.bindFramebuffer();
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	gl::draw( posTexture );
	fbo.unbindFramebuffer();
}

void MAPSS::setFboVelocities( gl::Fbo fbo ) {
	// FISH VELOCITY
	Surface32f velSurface( fbo.getTexture() );
	Surface32f::Iter it = velSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			float per		= (float)it.x()/(float)it.getWidth();
			float angle		= per * M_PI * 2.0f;
			float cosA		= cos( angle );
			float sinA		= sin( angle );
			Vec3f p			= Vec3f( cosA, 0.0f, sinA );
			it.r() = p.x;
			it.g() = p.y;
			it.b() = p.z;
			it.a() = 1.0f;
		}
	}
	
	gl::Texture velTexture( velSurface );
	fbo.bindFramebuffer();
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	gl::draw( velTexture );
	fbo.unbindFramebuffer();
}

void MAPSS::initVbo() {
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();
	
	int numVertices = mFboDim * mFboDim;
	
	mVboMesh		= gl::VboMesh( numVertices * 8 * 3, 0, layout, GL_TRIANGLES );
	
	float s = 1.5f;
	Vec3f p0( 0.0f, 0.0f, 2.0f );
	Vec3f p1( -s, -s, 0.0f );
	Vec3f p2( -s,  s, 0.0f );
	Vec3f p3(  s,  s, 0.0f );
	Vec3f p4(  s, -s, 0.0f );
	Vec3f p5( 0.0f, 0.0f, -5.0f );
	
	Vec3f n;
	
	vector<Vec3f>		positions;
	vector<Vec3f>		normals;
	vector<Vec2f>		texCoords;
	
	for( int x = 0; x < mFboDim; ++x ) {
		for( int y = 0; y < mFboDim; ++y ) {
			float u = (float)x/(float)mFboDim;
			float v = (float)y/(float)mFboDim;
			Vec2f t = Vec2f( u, v );
			
			positions.push_back( p0 );
			positions.push_back( p1 );
			positions.push_back( p2 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p1 + p2 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p2 );
			positions.push_back( p3 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p2 + p3 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p3 );
			positions.push_back( p4 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p3 + p4 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p0 );
			positions.push_back( p4 );
			positions.push_back( p1 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p0 + p4 + p1 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p1 );
			positions.push_back( p4 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p1 + p4 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p2 );
			positions.push_back( p1 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p2 + p1 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p3 );
			positions.push_back( p2 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p3 + p2 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
			
			positions.push_back( p5 );
			positions.push_back( p4 );
			positions.push_back( p3 );
			texCoords.push_back( t );
			texCoords.push_back( t );
			texCoords.push_back( t );
			n = ( p5 + p4 + p3 ).normalized();
			normals.push_back( n );
			normals.push_back( n );
			normals.push_back( n );
		}
	}
	
	mVboMesh.bufferPositions( positions );
	mVboMesh.bufferTexCoords2d( 0, texCoords );
	mVboMesh.bufferNormals( normals );
	mVboMesh.unbindBuffers();
}

void MAPSS::mouseDown( MouseEvent event ) {
	mMouseDownPos = event.getPos();
	mMousePressed = true;
	mMouseOffset = Vec2f::zero();
}

void MAPSS::mouseUp( MouseEvent event ) {
	mMousePressed = false;
	mMouseOffset = Vec2f::zero();
}

void MAPSS::mouseMove( MouseEvent event ) {
	mMousePos = event.getPos();
}

void MAPSS::mouseDrag( MouseEvent event ) {
	mouseMove( event );
	mMouseOffset = ( mMousePos - mMouseDownPos );
}

void MAPSS::mouseWheel( MouseEvent event ) {
	float dWheel = event.getWheelIncrement();
	mRoom.adjustTimeMulti( dWheel );
}

void MAPSS::keyDown( KeyEvent event ) {
}

void MAPSS::update() {
	processGestures();

	if( !mInitUpdateCalled ){
		mInitUpdateCalled = true;
	}
	
	// ROOM
	mRoom.update();
	
	// CONTROLLER
	mController.update();

	// CAMERA
	if( mMousePressed ) {
		mSpringCam.dragCam( ( mMouseOffset ) * 0.01f, ( mMouseOffset ).length() * 0.01f );
	}

	mSpringCam.update( 0.3f );
	
	gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::color( Color( 1, 1, 1 ) );

	drawIntoVelocityFbo();
	drawIntoPositionFbo();
	drawIntoRoomFbo();
	drawIntoLanternsFbo();
}


// FISH VELOCITY
void MAPSS::drawIntoVelocityFbo() {
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	
	mVelocityFbos[ mThisFbo ].bindFramebuffer();
	gl::clear( ColorA( 0, 0, 0, 0 ) );
	
	mPositionFbos[ mPrevFbo ].bindTexture( 0 );
	mVelocityFbos[ mPrevFbo ].bindTexture( 1 );
	mP_PositionFbos[ mPrevFbo ].bindTexture( 2 );
	mLanternsFbo.bindTexture( 3 );
	
	mVelocityShader.bind();
	mVelocityShader.uniform( "positionTex", 0 );
	mVelocityShader.uniform( "velocityTex", 1 );
	mVelocityShader.uniform( "lanternsTex", 3 );
	mVelocityShader.uniform( "numLights", (float)mController.mNumLanterns );
	mVelocityShader.uniform( "invNumLights", 1.0f/(float)MAX_LANTERNS );
	mVelocityShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_LANTERNS * 0.5f );
	mVelocityShader.uniform( "att", 1.015f );
	mVelocityShader.uniform( "roomBounds", mRoom.getDims() );
	mVelocityShader.uniform( "fboDim", mFboDim );
	mVelocityShader.uniform( "invFboDim", 1.0f/(float)mFboDim );
	mVelocityShader.uniform( "pFboDim", mP_FboDim );
	mVelocityShader.uniform( "pInvFboDim", 1.0f/(float)mP_FboDim );
	mVelocityShader.uniform( "dt", mRoom.mTimeAdjusted );
	mVelocityShader.uniform( "power", mRoom.getPower() );
	gl::drawSolidRect( mFboBounds );
	mVelocityShader.unbind();
	
	mVelocityFbos[ mThisFbo ].unbindFramebuffer();
}

// FISH POSITION
void MAPSS::drawIntoPositionFbo() {
	gl::setMatricesWindow( mFboSize, false );
	gl::setViewport( mFboBounds );
	
	mPositionFbos[ mThisFbo ].bindFramebuffer();
	mPositionFbos[ mPrevFbo ].bindTexture( 0 );
	mVelocityFbos[ mThisFbo ].bindTexture( 1 );
	
	mPositionShader.bind();
	mPositionShader.uniform( "position", 0 );
	mPositionShader.uniform( "velocity", 1 );
	mPositionShader.uniform( "dt", mRoom.mTimeAdjusted );
	gl::drawSolidRect( mFboBounds );
	mPositionShader.unbind();
	
	mPositionFbos[ mThisFbo ].unbindFramebuffer();
}

void MAPSS::drawIntoRoomFbo() {
	gl::setMatricesWindow( mRoomFbo.getSize(), false );
	gl::setViewport( mRoomFbo.getBounds() );
	
	mRoomFbo.bindFramebuffer();
	gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ), true );
	
	gl::disableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	Matrix44f m;
	m.setToIdentity();
	m.scale( mRoom.getDims() );
	
	mLanternsFbo.bindTexture();
	mRoomShader.bind();
	mRoomShader.uniform( "lanternsTex", 0 );
	mRoomShader.uniform( "numLights", (float)mController.mNumLanterns );
	mRoomShader.uniform( "invNumLights", 1.0f/(float)MAX_LANTERNS );
	mRoomShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_LANTERNS * 0.5f );
	mRoomShader.uniform( "att", 1.25f );
	mRoomShader.uniform( "mvpMatrix", mSpringCam.mMvpMatrix );
	mRoomShader.uniform( "mMatrix", m );
	mRoomShader.uniform( "eyePos", mSpringCam.mEye );
	mRoomShader.uniform( "roomDims", mRoom.getDims() );
	mRoomShader.uniform( "power", mRoom.getPower() );
	mRoomShader.uniform( "lightPower", mRoom.getLightPower() );
	mRoomShader.uniform( "timePer", mRoom.getTimePer() * 1.5f + 0.5f );
	mRoom.draw();
	mRoomShader.unbind();
	
	mRoomFbo.unbindFramebuffer();
	glDisable( GL_CULL_FACE );
}

void MAPSS::draw() {
	if( !mInitUpdateCalled ){
		return;
	}

	gl::clear( ColorA( 0.1f, 0.1f, 0.1f, 0.0f ), true );
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
	
	gl::setMatricesWindow( getWindowSize(), false );
	gl::setViewport( getWindowBounds() );
	
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::enable( GL_TEXTURE_2D );
	gl::enableAlphaBlending();
	
	// DRAW ROOM
	mRoomFbo.bindTexture();
	gl::drawSolidRect( getWindowBounds() );
	
	gl::setMatrices( mSpringCam.getCam() );
	gl::setViewport( getWindowBounds() );
	
	gl::enableAlphaBlending();
	gl::enable( GL_TEXTURE_2D );
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	// DRAW PARTICLES
	mPositionFbos[mPrevFbo].bindTexture( 0 );
	mPositionFbos[mThisFbo].bindTexture( 1 );
	mVelocityFbos[mThisFbo].bindTexture( 2 );
	mLanternsFbo.bindTexture( 3 );
	mShader.bind();
	mShader.uniform( "prevPosition", 0 );
	mShader.uniform( "currentPosition", 1 );
	mShader.uniform( "currentVelocity", 2 );
	mShader.uniform( "lightsTex", 3 );
	mShader.uniform( "numLights", (float)mController.mNumLanterns );
	mShader.uniform( "invNumLights", 1.0f/(float)MAX_LANTERNS );
	mShader.uniform( "invNumLightsHalf", 1.0f/(float)MAX_LANTERNS * 0.5f );
	mShader.uniform( "att", 1.05f );
	mShader.uniform( "eyePos", mSpringCam.mEye );
	mShader.uniform( "power", mRoom.getPower() );
	gl::draw( mVboMesh );
	mShader.unbind();
	
	// DRAW LANTERN GLOWS
    gl::disableDepthWrite();
    gl::enableAdditiveBlending();
    float c =  mRoom.getPower();
    gl::color( Color( c, c, c ) );
    mLanternGlowTex.bind();
    mController.drawLanternGlows( mSpringCam.mBillboardRight, mSpringCam.mBillboardUp );
	
	gl::disable( GL_TEXTURE_2D );
	gl::enableDepthWrite();
	gl::enableAdditiveBlending();
	gl::color( Color( 1.0f, 1.0f, 1.0f ) );
	
	// DRAW LANTERNS
	mLanternShader.bind();
	mLanternShader.uniform( "mvpMatrix", mSpringCam.mMvpMatrix );
	mLanternShader.uniform( "eyePos", mSpringCam.mEye );
	mLanternShader.uniform( "mainPower", mRoom.getPower() );
	mLanternShader.uniform( "roomDim", mRoom.getDims() );
	mController.drawLanterns( &mLanternShader );
	mLanternShader.unbind();

	gl::disableDepthWrite();
	gl::enableAlphaBlending();
	
    // DRAW POSITION AND VELOCITY FBOS
	if( false ){
		gl::color( Color::white() );
		gl::setMatricesWindow( getWindowSize() );
		gl::enable( GL_TEXTURE_2D );
		mPositionFbos[ mThisFbo ].bindTexture();
		gl::drawSolidRect( Rectf( 5.0f, 5.0f, 105.0f, 105.0f ) );
		
		mPositionFbos[ mPrevFbo ].bindTexture();
		gl::drawSolidRect( Rectf( 106.0f, 5.0f, 206.0f, 105.0f ) );
		
		mVelocityFbos[ mThisFbo ].bindTexture();
		gl::drawSolidRect( Rectf( 5.0f, 106.0f, 105.0f, 206.0f ) );
		
		mVelocityFbos[ mPrevFbo ].bindTexture();
		gl::drawSolidRect( Rectf( 106.0f, 106.0f, 206.0f, 206.0f ) );
	}
	
	mThisFbo	= ( mThisFbo + 1 ) % 2;
	mPrevFbo	= ( mThisFbo + 1 ) % 2;	
}

// HOLDS DATA FOR LANTERNS AND PREDATORS
void MAPSS::drawIntoLanternsFbo() {
	Surface32f lanternsSurface( mLanternsFbo.getTexture() );
	Surface32f::Iter it = lanternsSurface.getIter();
	while( it.line() ){
		while( it.pixel() ){
			int index = it.x();
			
			if( it.y() == 0 ){ // set light position
				if( index < (int)mController.mLanterns.size() ){
					it.r() = mController.mLanterns[index].mPos.x;
					it.g() = mController.mLanterns[index].mPos.y;
					it.b() = mController.mLanterns[index].mPos.z;
					it.a() = mController.mLanterns[index].mRadius;
				} else { // if the light shouldnt exist, put it way out there
					it.r() = 0.0f;
					it.g() = 0.0f;
					it.b() = 0.0f;
					it.a() = 1.0f;
				}
			} else {	// set light color
				if( index < (int)mController.mLanterns.size() ){
					it.r() = mController.mLanterns[index].mColor.r;
					it.g() = mController.mLanterns[index].mColor.g;
					it.b() = mController.mLanterns[index].mColor.b;
					it.a() = 1.0f;
				} else { 
					it.r() = 0.0f;
					it.g() = 0.0f;
					it.b() = 0.0f;
					it.a() = 1.0f;
				}
			}
		}
	}

	mLanternsFbo.bindFramebuffer();
	gl::setMatricesWindow( mLanternsFbo.getSize(), false );
	gl::setViewport( mLanternsFbo.getBounds() );
	gl::draw( gl::Texture( lanternsSurface ) );
	mLanternsFbo.unbindFramebuffer();
}

CINDER_APP_BASIC( MAPSS, RendererGl )
