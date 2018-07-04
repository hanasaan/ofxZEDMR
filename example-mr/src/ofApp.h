#pragma once

#include "ofMain.h"
#include "ofxOpenVR.h"
#include "ofxZED.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void exit();

		void update();
		void draw();

		void render(vr::Hmd_Eye nEye);

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
		ofxZED zed;

		ofQuaternion hmdPlaneRot;
		ofVec3f hmdPlanePos;

		ofVec2f renderPlaneSize;
		float HMDFocal;
		float EyeToZedDistance = 0.0f;
		ofVec2f OffCenterProjectionOffset;
		float PlaneDistance = 10.0; // from ZEDMixedRealityPlugin.cs
		ofVec3f plane_offset = ofVec3f(0, 0, -PlaneDistance);
		sl::timeStamp image_ts;

		ofShader bgra_shader;
		bool zed_tracking_enable = false; // ToDo

		ofMatrix4x4 quad_left;
		ofMatrix4x4 quad_right;

};
