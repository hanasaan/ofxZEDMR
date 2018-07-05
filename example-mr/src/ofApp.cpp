#include "ofApp.h"


#define STRINGIFY(A) #A

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	ofDisableArbTex();

	// We need to pass the method we want ofxOpenVR to call when rending the scene
	openVR.setup(std::bind(&ofApp::render, this, std::placeholders::_1));
	zed.init(true, true, true, 0);
	mr.setup(&zed, &openVR, ofxZED::HmdType_Vive);
}

//--------------------------------------------------------------
void ofApp::exit() {
	openVR.exit();
	mr.close();
}


//--------------------------------------------------------------
void ofApp::update(){
	openVR.update();
	zed.update();
	mr.update();
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
	//ofLoadMatrix(currentViewMatrixInvertY);
	ofLoadViewMatrix(currentViewMatrixInvertY);

	ofPushStyle();
	ofSetColor(ofColor::white);

	ofPushMatrix();
	ofRotateZ(90);
	ofDrawGridPlane(0.125, 10);
	ofPopMatrix();

	mr.drawCameraTexture((ofxZED::VREye&)nEye);

	ofPopStyle();

	ofPopView();
}


//--------------------------------------------------------------
void ofApp::draw(){
	openVR.render();
	openVR.renderDistortion();

	openVR.drawDebugInfo(10.0f, 500.0f);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
