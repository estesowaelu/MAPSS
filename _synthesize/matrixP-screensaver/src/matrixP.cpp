#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "ParticleController.h"

#define RESOLUTION 15
#define WIDTH 640
#define HEIGHT 761

using namespace ci;
using namespace ci::app;
using namespace std;

class matrixP : public AppBasic {
  public:
	void prepareSettings(Settings *settings);
	void setup();
	void update();
	void draw();
	void keyDown(KeyEvent event);
	void mouseMove(MouseEvent event);
	void mouseDrag(MouseEvent event);
	
	ParticleController mParticleController;
	
	Vec2i mMouseLoc;
	
	bool mRenderParticles;
	int mRenderColor;
	bool mRenderOrdered;
};

void matrixP::prepareSettings(Settings *settings){
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setFrameRate(60.0f);
}

void matrixP::setup() {
	mParticleController = ParticleController(RESOLUTION);
	mMouseLoc = Vec2i(0, 0);
	mRenderParticles = TRUE;
	mRenderColor = 1;
	mRenderOrdered = FALSE;
}

void matrixP::update() {
	mParticleController.update(mRenderColor, mRenderOrdered, mMouseLoc);
}

void matrixP::draw() {
	gl::clear();
	if(mRenderParticles) mParticleController.draw();
}

void matrixP::keyDown(KeyEvent event) {
	// show-hide particles
	if(event.getChar() == '0') mRenderParticles = !mRenderParticles;
	// greyscale
	if(event.getChar() == '1') mRenderColor = 1;
	// color
	if(event.getChar() == '2') mRenderColor = 2;
	// gradient
	if(event.getChar() == '3') mRenderColor = 3;
	// follow-decay
	if(event.getChar() == '4') mRenderOrdered = !mRenderOrdered;
}

void matrixP::mouseMove(MouseEvent event) {
	mMouseLoc = event.getPos();
}

void matrixP::mouseDrag(MouseEvent event) {
	mouseMove(event);
}

CINDER_APP_BASIC(matrixP, RendererGl)
