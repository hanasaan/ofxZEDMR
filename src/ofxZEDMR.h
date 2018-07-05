//
//  Created by Yuya Hanai, https://github.com/hanasaan/
//
#pragma once

#include "ofMain.h"
#include "ofxZED.h"
#include "ofxOpenVR.h"

#include <sl_mr_core/defines.hpp>

namespace ofxZED
{
	static const float PLANE_DISTANCE = 10.0f;

	enum HmdType
	{
		HmdType_Vive = 0,
		HmdType_Oculus = 1
	};

	enum VREye
	{
		Eye_Left = 0,
		Eye_Right = 1
	};

	class MR
	{
	public:
		MR() {}
		~MR() { close(); }

		void setup(Camera * camera, ofxOpenVR * openvr, HmdType hmdtype);
		void update();
		void close();

		void drawCameraTexture(VREye eye);
	protected:
		void loadHmdToZEDCalibration();
		void setupRenderPlane();
		void initTrackingAR();

		void updateHmdPose();
		void extractLatencyPose();
		void adjustTrackingAR();
		void updateTracking();
		void lateUpdateHmdRendering();

		Camera * camera = nullptr;
		ofxOpenVR * openvr = nullptr;

		ofVec2f render_plane_size;
		ofVec3f plane_offset = ofVec3f(0, 0, -PLANE_DISTANCE);

		ofShader bgra_shader;

		ofMatrix4x4 quad_left;
		ofMatrix4x4 quad_right;

		sl::Transform latency_pose;
		bool b_latency_pose_ready = false;

		sl::mr::trackingData tracking_data;
		sl::Transform zed_transform;

		sl::Transform zed_rig_root;
		sl::Transform calib_transform;
	};
}


