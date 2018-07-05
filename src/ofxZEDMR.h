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

	class CustomProjectionMatrixCamera : public ofCamera
	{
	protected:
		ofMatrix4x4 projection;
	public:
		void setup(float w, float h, float fx, float fy, float cx, float cy);
		void begin(const ofRectangle & viewport) override;
		void begin() override;
		void end() override;
	};

	class MRCamFbo
	{
	public:
		CustomProjectionMatrixCamera cam;
		ofFbo fbo;

		void setup(Camera * camera, VREye eye, int internal_format = GL_RGB, int num_samples = 0);
		void begin()
		{
			fbo.begin();
			ofClear(0);
			cam.begin();
		}

		void end()
		{
			cam.end();
			fbo.end();
		}
	};

	class MR
	{
	public:
		MR() {}
		~MR() { close(); }

		void setup(Camera * camera, ofxOpenVR * openvr, HmdType hmdtype, std::function< void(VREye) > f);
		void update();
		void close();

		void renderScene();

		void drawCameraTexture(VREye eye);

		void drawCameraTexture2D(VREye eye);
		void drawRenderedSceneTexture(VREye eye);

		// for camera texture rendering
		ofShader& getBgraShader() { return  bgra_shader;  }

		MRCamFbo& getCamLeft() { return cam_left; }
		MRCamFbo& getCamRight() { return cam_right; }
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

		ofMatrix4x4 quad2d_left;
		ofMatrix4x4 quad2d_right;

		sl::Transform latency_pose;
		bool b_latency_pose_ready = false;

		sl::mr::trackingData tracking_data;
		sl::Transform zed_transform;

		sl::Transform zed_rig_root;
		sl::Transform calib_transform;

		MRCamFbo cam_left;
		MRCamFbo cam_right;

		std::function< void(VREye) > _callable_render_function;
	};
}

