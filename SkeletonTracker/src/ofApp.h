#pragma once

#define USE_PROGRAMMABLE_PIPELINE 1

#include "ofMain.h"
#include "ofxKinectForWindows2.h"
#include "ofxGui.h"
#include "ThreadedClient.h"
#include "FilteredPoint.h"
#include "ofxOsc.h"

#include "body.pb.h"

using namespace srl::body;


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);


		// ip_addr and port for broadcasting data
		vector<string> args; 
		bool load_params_from_file = true;

		// ------------ GTC 2019 -------------
		ofBoxPrimitive interaction_zone;
		float crouch_scalar = 1;
		ofxOscSender sender;
		void setup_osc();

		FilteredPoint filter_crouch;
		 

		// ---------- SENSOR SETUP -----------
		void setup_sensor();
		void update_sensor();
		ofxKFW2::Device kinect;

		// --------- 3D WORLD & NAV ----------
		void setup_world();
		void draw_world();
		ofEasyCam cam;
		//ofMesh mirror_plane;
		//float mirror_plane_width = 3;
		//float mirror_plane_height = 2;

		void setup_shader();
		//void render_shader(); // <-- can't be rendered outside of draw() function for some reason ???
		ofShader shader;

		// ---------- YuMi PARAMS ------------
		//FilteredPoint fp_tgt_left, fp_tgt_right;
		//ofNode target_left, target_right;
		//ofCylinderPrimitive arm_left, arm_right;
		//ofSpherePrimitive hand_sphere_left, hand_sphere_right;

		void update_bodies();
		void update_body(ofxKinectForWindows2::Data::Body _body);
		map<int, Body*> bodies;

		// Debugging Calibrartion
		void draw_body(Body* _body);
		void draw_joints(Body* _body);
		void draw_skeleton(Body* _body);
		float scalar = 1000.;

		void update_closest_body();
		void generate_robot_targets();
		void update_robot_targets();
		Body* closest_body;
		Body default_body;

		

		// --------------- COM ---------------
		void setup_client();
		ThreadedClient client;

		// --------------- GUI ---------------
		void setup_gui();
		bool show_gui = true;
		ofxPanel panel;
		ofParameterGroup params;
		ofParameter<bool> show_mesh_faces;
		ofParameter<bool> show_skeletons;
		ofParameter<bool> use_world_coordinates;
		ofParameter<bool> do_streaming;

		ofParameterGroup params_sensor;
		ofParameter<ofVec3f> sensor_offset;
		ofParameter<float> sensor_tilt;

		ofParameterGroup params_interaction;
		ofParameter<bool> idle, mirror, avoid, other;
		ofParameter<ofVec3f> interaction_zone_offset;
		ofParameter<float> crouch_dist_min;
		ofParameter<float> crouch_dist_max;
		ofParameter<bool> show_crouch;
		//ofParameter<ofVec3f> robot_bounds_min;
		//ofParameter<ofVec3f> robot_bounds_max;
		//ofBoxPrimitive robot_bounds;

		//ofBoxPrimitive avoid_zone, mirror_zone;

		void listener_idle(bool &val);
		void listener_mirror(bool &val);
		void listener_avoid(bool &val);
		void listener_other(bool &val);
		void listener_interaction_zone_offset(ofVec3f &val);
		//void listener_mirror_plane_offset(ofVec3f &val);
		//void listener_robot_bounds(ofVec3f &val);

	private:

		ofNode toOf(Pose* pose);
		bool isInside(ofVec3f pt, ofBoxPrimitive aabb);

		void handle_keypressed_cam(int key);
		void listener_show_top(bool & val);
		void listener_show_front(bool & val);
		void listener_show_side(bool & val);
		void listener_show_perspective(bool & val);

		void draw_frustum();

		vector<string> joint_names = {
			"SPINE_BASE",
			"SPINE_MID",
			"NECK",
			"HEAD",
			"SHOULDER_LEFT",
			"ELBOW_LEFT",
			"WRIST_LEFT",
			"HAND_LEFT",
			"SHOULDER_RIGHT",
			"ELBOW_RIGHT",
			"WRIST_RIGHT",
			"HAND_RIGHT",
			"HIP_LEFT",
			"KNEE_LEFT" ,
			"ANKLE_LEFT",
			"FOOT_LEFT",
			"HIP_RIGHT",
			"KNEE_RIGHT",
			"ANKLE_RIGHT",
			"FOOT_RIGHT",
			"SPINE_SHOULDER",
			"HAND_TIP_LEFT",
			"THUMB_LEFT",
			"HAND_TIP_RIGHT",
			"THUMB_RIGHT"
		};
		enum {
			SPINE_BASE,
			SPINE_MID,
			NECK,
			HEAD,
			SHOULDER_LEFT,
			ELBOW_LEFT,
			WRIST_LEFT,
			HAND_LEFT,
			SHOULDER_RIGHT,
			ELBOW_RIGHT,
			WRIST_RIGHT,
			HAND_RIGHT,
			HIP_LEFT,
			KNEE_LEFT,
			ANKLE_LEFT,
			FOOT_LEFT,
			HIP_RIGHT,
			KNEE_RIGHT,
			ANKLE_RIGHT,
			FOOT_RIGHT,
			SPINE_SHOULDER,
			HAND_TIP_LEFT,
			THUMB_LEFT,
			HAND_TIP_RIGHT,
			THUMB_RIGHT
		};

};


/*
JointTypes From Kinect.h

enum _JointType
{
JointType_SpineBase	= 0,
JointType_SpineMid	= 1,
JointType_Neck	= 2,
JointType_Head	= 3,
JointType_ShoulderLeft	= 4,
JointType_ElbowLeft	= 5,
JointType_WristLeft	= 6,
JointType_HandLeft	= 7,
JointType_ShoulderRight	= 8,
JointType_ElbowRight	= 9,
JointType_WristRight	= 10,
JointType_HandRight	= 11,
JointType_HipLeft	= 12,
JointType_KneeLeft	= 13,
JointType_AnkleLeft	= 14,
JointType_FootLeft	= 15,
JointType_HipRight	= 16,
JointType_KneeRight	= 17,
JointType_AnkleRight	= 18,
JointType_FootRight	= 19,
JointType_SpineShoulder	= 20,
JointType_HandTipLeft	= 21,
JointType_ThumbLeft	= 22,
JointType_HandTipRight	= 23,
JointType_ThumbRight	= 24,
JointType_Count	= ( JointType_ThumbRight + 1 )
} ;

*/