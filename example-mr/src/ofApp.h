#pragma once

#include "ofMain.h"
#include "ofxOpenVR.h"
#include "ofxZED.h"
#include "ofxZEDMR.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void exit();

		void update();
		void draw();

		void renderMR(ofxZED::VREye eye);
		void render(vr::Hmd_Eye nEye);

		void drawScene();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofxOpenVR openVR;
		ofxZED::Camera zed;
		ofxZED::MR mr;

		ofEasyCam ecam;

		ofMatrix4x4 lcp;
		ofMatrix4x4 rcp;

		bool b_draw_individual = false;
};