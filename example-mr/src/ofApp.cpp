#include "ofApp.h"

#include <sl_mr_core/Rendering.hpp>
#include <sl_mr_core/latency.hpp>
#include <sl_mr_core/antidrift.hpp>

#define STRINGIFY(A) #A

namespace ofxZEDUtil
{
	inline ofMatrix4x4 toOf(const sl::Transform& t)
	{
		return ofMatrix4x4(&t.m[0]);
	}

	inline sl::Transform toZed(ofMatrix4x4& mat)
	{
		return sl::Transform(mat.getPtr());
	}

	inline sl::Transform toZed(glm::mat4& mat)
	{
		return sl::Transform(reinterpret_cast<float*>(&mat));
	}

	inline ofVec2f toOf(const sl::float2& v)
	{
		return ofVec2f(v.x, v.y);
	}

	inline ofVec3f toOf(const sl::float3& v)
	{
		return ofVec3f(v.x, v.y, v.z);
	}

	inline ofVec4f toOf(const sl::float4& v)
	{
		return ofVec4f(v.x, v.y, v.z, v.w);
	}

	inline sl::float2 toZed(const ofVec2f& v)
	{
		return sl::float2(v.x, v.y);
	}

}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(false);
	ofDisableArbTex();

	// We need to pass the method we want ofxOpenVR to call when rending the scene
	openVR.setup(std::bind(&ofApp::render, this, std::placeholders::_1));

	zed.init(true, false, 0);

	sl::mr::latencyCorrectorInitialize(sl::mr::eHmdType_Vive);
	sl::mr::driftCorrectorInitialize();

	sl::Transform calib_transform;
	calib_transform.setTranslation(sl::Translation(0.0315f, 0, 0.11f));
	sl::mr::driftCorrectorSetCalibrationTransform(calib_transform);

	// Only for vive... some image adjustment
	if (true) {
		zed.getZedCamera()->setCameraSettings(sl::CAMERA_SETTINGS::CAMERA_SETTINGS_CONTRAST, 3);
		zed.getZedCamera()->setCameraSettings(sl::CAMERA_SETTINGS::CAMERA_SETTINGS_SATURATION, 3);
	}

	string vert150 = "#version 150\n";
	vert150 += STRINGIFY
	(
		in vec4 position;
		in vec2 texcoord;

		out vec2 texCoordVarying;

		uniform mat4 modelViewProjectionMatrix;

		void main()
		{
			texCoordVarying = texcoord;
			gl_Position = modelViewProjectionMatrix * position;
		}
	);

	string frag150 = "#version 150\n";
	frag150 += STRINGIFY
	(
		in vec2 texCoordVarying;
		out vec4 fragColor;
		uniform sampler2D tex;

		void main(void) {
			vec2 texcoord0 = texCoordVarying.xy;
			vec4 tmp = texture(tex, texcoord0);
			fragColor = vec4(tmp.b, tmp.g, tmp.r, tmp.a);
		}
	);

	bgra_shader.setupShaderFromSource(GL_VERTEX_SHADER, vert150);
	bgra_shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag150);
	bgra_shader.bindDefaults();
	bgra_shader.linkProgram();
}

//--------------------------------------------------------------
void ofApp::exit() {
	openVR.exit();

	sl::mr::driftCorrectorShutdown();
	sl::mr::latencyCorrectorShutdown();
}

//--------------------------------------------------------------
void ofApp::update(){
	openVR.update();
	zed.update();

	// HMD tracking transform
	sl::Transform SlHMDTransform = ofxZEDUtil::toZed(openVR.getmat4HMDPose());

	// Latency corrector if new image
	// Add key pose
	// { // UpdateHmdPose
	sl::timeStamp CurrentTimestamp = zed.getZedCamera()->getTimestamp(sl::TIME_REFERENCE::TIME_REFERENCE_CURRENT);
	sl::mr::latencyCorrectorAddKeyPose(sl::mr::keyPose(SlHMDTransform, CurrentTimestamp));
	// } // UpdateHmdPose

	// { // UpdateTracking()
	// Latency corrector
	if (zed_tracking_enable) {
		// toDo
		//ar.ExtractLatencyPose(imageTimeStamp);
		//ar.AdjustTrackingAR(zedPosition, zedOrientation, out r, out v);
	}
	else {
		// latency transform
		sl::Transform SlLatencyTransform;
		if (zed.isFrameNew()) {
			image_ts = zed.getZedCamera()->getTimestamp(sl::TIME_REFERENCE::TIME_REFERENCE_IMAGE);
		}
		sl::mr::latencyCorrectorGetTransform(image_ts, SlLatencyTransform);

		this->hmdPlaneRot = ofxZEDUtil::toOf(SlLatencyTransform).getRotate();
		this->hmdPlanePos = ofxZEDUtil::toOf(SlLatencyTransform).getTranslation();
	}

	// } // UpdateTracking()

	// Update render plane size
	// obtained from ZEDMixedRealityPlugin.cs
	EyeToZedDistance = 0.1;// sl::mr::getEyeToZEDDistance(sl::mr::HMD_DEVICE_TYPE::HMD_DEVICE_TYPE_HTC);
	sl::Resolution HMDScreenResolution;
	HMDScreenResolution.width = 3024;
	HMDScreenResolution.height = 1680;

	auto projectionmat = openVR.getCurrentProjectionMatrix(vr::Eye_Left);
	HMDFocal = sl::mr::computeHMDFocal(HMDScreenResolution, projectionmat[0][0], projectionmat[1][1]);

	float PerceptionDistance = 1.0;
	float ZedFocal = zed.getZedCamera()->getCameraInformation().calibration_parameters.left_cam.fx;
	sl::Resolution res;
	res.width = zed.zedWidth;
	res.height = zed.zedHeight;
	this->renderPlaneSize = ofxZEDUtil::toOf(sl::mr::computeRenderPlaneSizeWithGamma(res, PerceptionDistance, EyeToZedDistance, PlaneDistance, HMDFocal, ZedFocal));


	//Quaternion r;
	//r = latencyPose.rotation;
	quad_left.setRotate(hmdPlaneRot);
	quad_left.setTranslation(plane_offset * ofMatrix4x4(hmdPlaneRot));

	quad_right.setRotate(hmdPlaneRot);
	quad_right.setTranslation(plane_offset * ofMatrix4x4(hmdPlaneRot));

	//quadLeft.localRotation = r;
	//quadLeft.localPosition = finalLeftEye.transform.localPosition + r * (offset);
	//quadRight.localRotation = r;
	//quadRight.localPosition = finalRightEye.transform.localPosition + r * (offset);

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

	ofLoadViewMatrix(ofMatrix4x4());
	ofPushMatrix();
	ofSetRectMode(OF_RECTMODE_CENTER);
	//ofTranslate(0, 0, -PlaneDistance);
	//ofMatrix4x4 m = ofMatrix4x4(hmdPlaneRot);
	ofMatrix4x4 m = openVR.getmat4HMDPose();
	ofMultMatrix(quad_left * ofMatrix4x4(m.getRotate()).getInverse());
	bgra_shader.begin();
	if (nEye == vr::Eye_Left) {
		zed.getColorLeftTexture().draw(0, 0, renderPlaneSize.x, renderPlaneSize.y);
	}
	else {
		zed.getColorRightTexture().draw(0, 0, renderPlaneSize.x, renderPlaneSize.y);
	}
	bgra_shader.end();
	ofSetRectMode(OF_RECTMODE_CORNER);
	ofPopMatrix();

	ofPopStyle();

	ofPopView();
}


//--------------------------------------------------------------
void ofApp::draw(){
	openVR.render();
	openVR.renderDistortion();

	openVR.drawDebugInfo(10.0f, 500.0f);

	{
		stringstream ss;
		ss << "hmdPlaneRot : " << hmdPlaneRot.getEuler() << endl;
		ss << "hmdPlanePos : " << hmdPlanePos << endl;
		ss << "renderPlaneSize : " << renderPlaneSize << endl;
		ss << "fx : " << zed.getZedCamera()->getCameraInformation().calibration_parameters.left_cam.fx << endl;
		ss << "fy : " << zed.getZedCamera()->getCameraInformation().calibration_parameters.left_cam.fy << endl;
		ss << "HMDFocal : " << HMDFocal << endl;
		ss << "EyeToZedDistance : " << EyeToZedDistance << endl;
		ss << "OffCenterProjectionOffset : " << OffCenterProjectionOffset << endl;
		ofDrawBitmapStringHighlight(ss.str(), 10, 20);
	}
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
