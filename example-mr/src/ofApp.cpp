#include "ofApp.h"


#define STRINGIFY(A) #A

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	ofDisableArbTex();

	// We need to pass the method we want ofxOpenVR to call when rending the scene
	openVR.setup(std::bind(&ofApp::render, this, std::placeholders::_1));
	zed.init(true, true, false, 0);
	mr.setup(&zed, &openVR, ofxZED::HmdType_Vive, std::bind(&ofApp::renderMR, this, std::placeholders::_1));
}

//--------------------------------------------------------------
void ofApp::exit() {
	openVR.exit();
	mr.close();
}


//--------------------------------------------------------------
void ofApp::update(){
	openVR.update();

	// some stuff
	if (openVR.isControllerConnected(vr::TrackedControllerRole_LeftHand)) {
		// Getting the translation component of the controller pose matrix
		lcp = openVR.getControllerPose(vr::TrackedControllerRole_LeftHand);
	}
	if (openVR.isControllerConnected(vr::TrackedControllerRole_RightHand)) {
		// Getting the translation component of the controller pose matrix
		rcp = openVR.getControllerPose(vr::TrackedControllerRole_RightHand);
	}


	mr.update(); // handles zed update inside mr...
	if (!ofGetKeyPressed(' ')) {
		mr.renderScene();
	}
}

void ofApp::renderMR(ofxZED::VREye eye)
{
	mr.drawCameraTexture2D(eye);
	drawScene();
}

//--------------------------------------------------------------
void  ofApp::render(vr::Hmd_Eye nEye) {
	// Using a shader
	ofPushView();
	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofLoadMatrix(openVR.getCurrentProjectionMatrix(nEye));
	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	ofMatrix4x4 currentViewMatrixInvertY = openVR.getCurrentViewMatrix(nEye);
	currentViewMatrixInvertY.scale(1.0f, -1.0f, 1.0f);
	ofLoadViewMatrix(currentViewMatrixInvertY);



	if (!ofGetKeyPressed(' ')) {
		mr.drawRenderedSceneTexture((ofxZED::VREye&)nEye);
	}
	else {
		drawScene();
		mr.drawCameraTexture((ofxZED::VREye&)nEye);
	}


	ofPopView();
}

void ofApp::drawScene()
{
	ofPushStyle();
	ofSetColor(ofColor::green);
	ofPushMatrix();
	ofRotateZ(90);
	ofDrawGridPlane(0.125, 10);
	ofPopMatrix();
	ofPopStyle();

	ofPushStyle();
	ofSetColor(ofColor::red);
	ofPushMatrix();
	ofMultMatrix(lcp);
	ofDrawBox(0.05);
	ofDrawAxis(0.2);
	ofPopMatrix();
	ofPopStyle();

	ofPushStyle();
	ofSetColor(ofColor::blue);
	ofPushMatrix();
	ofMultMatrix(rcp);
	ofDrawBox(0.05);
	ofDrawAxis(0.2);
	ofPopMatrix();
	ofPopStyle();
}


//--------------------------------------------------------------
void ofApp::draw(){
	openVR.render();
	openVR.renderDistortion();
	openVR.drawDebugInfo(10.0f, 500.0f);

	ofDrawBitmapStringHighlight(ofToString(mr.getLatencyOffset()), 10, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'i') {
		mr.initTrackingAR();
	}
	if (key == OF_KEY_UP) {
		mr.setLatencyOffset(mr.getLatencyOffset() + 1000 * 1000 * 5);
	}
	if (key == OF_KEY_DOWN) {
		mr.setLatencyOffset(mr.getLatencyOffset() - 1000 * 1000 * 5);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
