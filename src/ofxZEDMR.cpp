//
//  Created by Yuya Hanai, https://github.com/hanasaan/
//
#include "ofxZEDMR.h"

#include <sl_mr_core/Rendering.hpp>
#include <sl_mr_core/latency.hpp>
#include <sl_mr_core/antidrift.hpp>

#define STRINGIFY(A) #A

namespace ofxZED
{
	void CustomProjectionMatrixCamera::setup(float w, float h, float fx, float fy, float cx, float cy)
	{
		auto near_dist = getNearClip();
		auto far_dist = getFarClip();

		ofMatrix4x4 m;
		m.makeFrustumMatrix(near_dist * (-cx) / fx,
			near_dist * (w - cx) / fx,
			near_dist * (cy - h) / fy,
			near_dist * (cy) / fy,
			near_dist,
			far_dist);

		// flip Y-axis
		//m.postMultScale(ofVec3f(1, -1, 1));

		projection = m;
	}

	void CustomProjectionMatrixCamera::begin(const ofRectangle & viewport)
	{
		ofPushView();
		ofViewport(viewport);
		ofSetOrientation(OF_ORIENTATION_DEFAULT, this->isVFlipped());
		ofSetMatrixMode(OF_MATRIX_PROJECTION);
		ofLoadMatrix(projection);
		ofSetMatrixMode(OF_MATRIX_MODELVIEW);
		ofLoadViewMatrix(getModelViewMatrix());
	}

	void CustomProjectionMatrixCamera::begin()
	{
		begin(ofGetCurrentViewport());
	}

	void CustomProjectionMatrixCamera::end()
	{
		ofPopView();
	}

	void MRCamFbo::setup(Camera * camera, VREye eye, int internal_format, int num_samples)
	{
		fbo.allocate(camera->zedWidth, camera->zedHeight, internal_format, num_samples);
		fbo.begin();
		ofClear(200);
		fbo.end();

		auto p = camera->getZedCamera()->getCameraInformation().calibration_parameters;
		auto c = eye == Eye_Left ? p.left_cam : p.right_cam;

		cerr << "cx:" << c.cx << endl;
		cerr << "cy:" << c.cy << endl;
		cerr << "fx:" << c.fx << endl;
		cerr << "fy:" << c.fy << endl;
		cam.setNearClip(0.001);
		cam.setFarClip(100.0);
		cam.setup(camera->zedWidth, camera->zedHeight, c.fx, c.fy, c.cx, c.cy);
	}


	void MR::setup(Camera* camera, ofxOpenVR * openvr, HmdType hmdtype, std::function< void(VREye) > f)
	{
		this->camera = camera;
		this->openvr = openvr;
		this->_callable_render_function = f;

		sl::mr::latencyCorrectorInitialize((sl::mr::EHmdType&)hmdtype);
		sl::mr::driftCorrectorInitialize();

		// Only for vive... some image adjustment
		if (hmdtype == HmdType_Vive) {
			auto* zed = this->camera->getZedCamera();
			zed->setCameraSettings(sl::CAMERA_SETTINGS::CAMERA_SETTINGS_CONTRAST, 3);
			zed->setCameraSettings(sl::CAMERA_SETTINGS::CAMERA_SETTINGS_SATURATION, 3);
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

		// multisample fbo does not work..
		cam_left.setup(this->camera, Eye_Left, GL_RGB, 0);
		cam_right.setup(this->camera, Eye_Right, GL_RGB, 0);

		loadHmdToZEDCalibration();
		setupRenderPlane();

		openvr->update();
		initTrackingAR();
	}

	void MR::update()
	{
		updateHmdPose();
		camera->update();
		updateTracking();
		lateUpdateHmdRendering();
	}

	void MR::close()
	{
		sl::mr::driftCorrectorShutdown();
		sl::mr::latencyCorrectorShutdown();
	}

	void MR::renderScene()
	{
		cam_left.begin();
		_callable_render_function(Eye_Left);
		cam_left.end();

		cam_right.begin();
		_callable_render_function(Eye_Right);
		cam_right.end();
	}

	void MR::drawCameraTexture(VREye eye)
	{
		ofPushMatrix();

		auto mode = ofGetRectMode();
		ofSetRectMode(OF_RECTMODE_CENTER);
		bgra_shader.begin();
		if (eye == VREye::Eye_Left) {
			ofMultMatrix(quad_left);
			camera->getColorLeftTexture().draw(0, 0, render_plane_size.x, -render_plane_size.y);
		}
		else {
			ofMultMatrix(quad_right);
			camera->getColorRightTexture().draw(0, 0, render_plane_size.x, -render_plane_size.y);
		}
		bgra_shader.end();
		ofSetRectMode(mode);

		ofPopMatrix();
	}

	void MR::drawCameraTexture2D(VREye eye)
	{
		ofPushView();
		ofSetupScreen();
		bgra_shader.begin();
		if (eye == VREye::Eye_Left) {
			camera->getColorLeftTexture().draw(0, 0);
		}
		else {
			camera->getColorRightTexture().draw(0, 0);
		}
		bgra_shader.end();
		ofPopView();

	}

	void MR::drawRenderedSceneTexture(VREye eye)
	{
		ofPushMatrix();

		auto mode = ofGetRectMode();
		ofSetRectMode(OF_RECTMODE_CENTER);
		if (eye == VREye::Eye_Left) {
			ofMultMatrix(quad_left);
			cam_left.fbo.getTexture().draw(0, 0, render_plane_size.x, -render_plane_size.y);
		}
		else {
			ofMultMatrix(quad_right);
			cam_right.fbo.getTexture().draw(0, 0, render_plane_size.x, -render_plane_size.y);
		}
		ofSetRectMode(mode);

		ofPopMatrix();
	}
	
	void MR::loadHmdToZEDCalibration()
	{
		calib_transform.setTranslation(sl::Translation(-0.0315f, 0, -0.11f));
		sl::mr::driftCorrectorSetCalibrationTransform(calib_transform);
	}
	
	void MR::setupRenderPlane()
	{
		// magic parameters are obtained from ZEDMixedRealityPlugin.cs
		// https://github.com/stereolabs/zed-unity/
		auto eye2zed_distance = 0.1;
		sl::Resolution hmd_screen_resolution;
		hmd_screen_resolution.width = 3024;
		hmd_screen_resolution.height = 1680;

		auto projectionmat = openvr->getCurrentProjectionMatrix(vr::Eye_Left);
		auto hmd_focal = sl::mr::computeHMDFocal(hmd_screen_resolution, projectionmat[0][0], projectionmat[1][1]);

		auto perception_distance = 1.0;
		auto zed_focal = camera->getZedCamera()->getCameraInformation().calibration_parameters.left_cam.fx;
		sl::Resolution res;
		res.width = camera->zedWidth;
		res.height = camera->zedHeight;
		this->render_plane_size = ofxZED::toOf(sl::mr::computeRenderPlaneSizeWithGamma(res, 
			perception_distance, eye2zed_distance, PLANE_DISTANCE, hmd_focal, zed_focal));

	}
	
	void MR::initTrackingAR()
	{
		sl::Transform hmd_transform = ofxZED::toZed(openvr->getmat4HMDPose());

		sl::Transform const_transform;

		// which is correct... ??
		sl::mr::driftCorrectorSetConstOffsetTransfrom(const_transform); // from Unity Implementation
		//sl::mr::driftCorrectorSetConstOffsetTransfrom(hmd_transform * calib_transform); // from Unreal Implementation

		camera->getZedCamera()->resetTracking(hmd_transform * calib_transform);

		zed_rig_root = hmd_transform;
	}
	
	void MR::updateHmdPose()
	{
		sl::Transform hmd_transform = ofxZED::toZed(openvr->getmat4HMDPose());
		sl::timeStamp ts = camera->getZedCamera()->getTimestamp(sl::TIME_REFERENCE::TIME_REFERENCE_CURRENT);
		sl::mr::latencyCorrectorAddKeyPose(sl::mr::keyPose(hmd_transform, ts));
	}
	
	void MR::extractLatencyPose()
	{
		// latency transform
		sl::Transform latency_transform;
		if (sl::mr::latencyCorrectorGetTransform(camera->cameraTimestamp + latency_offset, latency_transform)) {
			this->latency_pose = latency_transform;
			b_latency_pose_ready = true;
		}
		else {
			b_latency_pose_ready = false;
		}
	}
	
	void MR::adjustTrackingAR()
	{
		tracking_data.trackingState = camera->trackingState;
		tracking_data.zedPathTransform = sl::Transform(camera->pose.getRotation(), camera->pose.getTranslation());

		if (camera->bZedReady && b_latency_pose_ready) {
			camera->getZedCamera()->setIMUPrior(latency_pose);
		}

		sl::Transform hmd_transform = ofxZED::toZed(openvr->getmat4HMDPose());
		sl::mr::driftCorrectorGetTrackingData(tracking_data, hmd_transform, latency_pose, true, true);
		zed_transform = tracking_data.zedWorldTransform;
	}
	
	void MR::updateTracking()
	{
		if (camera->bUseTracking) {
			extractLatencyPose();
			adjustTrackingAR();
			zed_rig_root = zed_transform * calib_transform;
		}
		else {
			extractLatencyPose();
			zed_rig_root = latency_pose * calib_transform;
		}
	}

	void MR::lateUpdateHmdRendering()
	{
		updateHmdPose();

		//
		// update render plane
		//
		auto r = ofxZED::toOf(latency_pose.getOrientation());
		auto pos = ofxZED::toOf(zed_rig_root.getTranslation());
		auto rot = ofxZED::toOf(zed_rig_root.getOrientation());

		auto baseline = camera->getZedCamera()->getCameraInformation().calibration_parameters.T[0];
		ofVec3f right_pos = ofVec3f(baseline, 0, 0) * ofxZED::toOf(zed_rig_root);

		quad_left.setRotate(r);
		quad_left.setTranslation(pos + plane_offset * ofMatrix4x4(r));
		quad2d_left.setRotate(rot);
		quad2d_left.setTranslation(pos + plane_offset * ofMatrix4x4(rot));


		quad_right.setRotate(r);
		quad_right.setTranslation(right_pos + plane_offset * ofMatrix4x4(r));
		quad2d_right.setRotate(rot);
		quad2d_right.setTranslation(right_pos + plane_offset * ofMatrix4x4(rot));

		cam_left.cam.setGlobalPosition(pos);
		cam_left.cam.setGlobalOrientation(rot);

		cam_right.cam.setGlobalPosition(right_pos);
		cam_right.cam.setGlobalOrientation(rot);
	}
}
