#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	// test argument passing
	//cout << "NUM args: " << args.size() << endl;
	//for (auto s : args) {
	//	cout << s << endl;
	//}

	setup_gui();

	setup_sensor();

	setup_shader();

	setup_world();

	setup_osc();
	setup_client();
}

//--------------------------------------------------------------
void ofApp::update() {
	
	update_sensor();

	update_bodies();
	bool is_crouching = false;
	if (bodies.size() > 0) {
		update_closest_body();

		// check if the closest body is within interaction range
		ofVec3f head = toOf(closest_body->mutable_joints(HEAD)->mutable_pose()).getGlobalPosition();
		if (isInside(head, interaction_zone)){
			generate_robot_targets();
			is_crouching = true;
		}
		// send an empty body
		else {
			closest_body = &default_body;
		}
	}
	else {
		closest_body = &default_body;
	}

	// send the closest body
	client.update_message(closest_body);


	// get the crouch percentage of the closest body
	if (is_crouching) {
		ofVec3f head = toOf(closest_body->mutable_joints(HEAD)->mutable_pose()).getGlobalPosition();
		ofVec3f butt = toOf(closest_body->mutable_joints(SPINE_BASE)->mutable_pose()).getGlobalPosition();

		// make Z height from FLOOR, not ORIGIN
		float floor_height = -2 * .254;
		float butt_to_floor_dist = butt.distance(ofVec3f(butt.x,butt.y,floor_height));

		float min = crouch_dist_min;// -floor_height;
		float max = crouch_dist_max;//  MAX(crouch_dist_max, butt.distance(head));
		cout << "distance from butt to ground: " << (butt_to_floor_dist) << endl;
		//cout << "distance from head to butt: " << butt.distance(head) << endl;
		
		
		crouch_scalar = ofMap(butt_to_floor_dist, min, max, 0, 1, true);
		cout << "\tmin: "<<min<< ", max: " << max<<", crouch_scalar: " << crouch_scalar << endl;
		
		if (do_streaming) {
			ofxOscMessage msg;
			msg.setAddress("/crouch_scalar");
			msg.addFloatArg(crouch_scalar);
			sender.sendMessage(msg);
		}

	}

}

//--------------------------------------------------------------
void ofApp::draw() {
	
	/* SENSOR & WORLD UNITS ARE IN METERS (M), BUT ARE DISPLAYED IN (MM) */

	
	/////////////////////////////////////////////////////////
	// DRAW 3D
	// Shader needs to be rendered directly in draw() (and first) for 
	// some reason ???
	/////////////////////////////////////////////////////////
	cam.begin();
	ofPushMatrix();
	if (use_world_coordinates) {
		ofRotateY(90);
		ofRotateZ(90);
	}
	// scale from M to MM
	ofScale(1000, 1000, 1000); 
	// translate based on Sensor Offset (in M) (... in world coordinates)
	ofTranslate(sensor_offset.get().y, sensor_offset.get().z, sensor_offset.get().x);
	ofRotateX(sensor_tilt);

	/* SHADER RENDERING NEEDS TO BE DIRECTLY IN DRAW() LOOP */
	// -------------- Render Mesh Shader ------------------
	shader.begin();
	shader.setUniform1i("uWidth", kinect.getBodyIndexSource()->getWidth());
	if (ofIsGLProgrammableRenderer()) {
		shader.setUniformTexture("uBodyIndexTex", kinect.getBodyIndexSource()->getTexture(), 1);
		shader.setUniformTexture("uColorTex", kinect.getColorSource()->getTexture(), 2);
	}
	else {
		// TEMP: Until OF master fixes texture binding for old pipeline.
		shader.setUniform1i("uBodyIndexTex", 1);
		kinect.getBodyIndexSource()->getTexture().bind(1);
		shader.setUniform1i("uColorTex", 2);
		kinect.getColorSource()->getTexture().bind(2);
	}

	ofSetColor(255);
	ofMesh mesh = kinect.getDepthSource()->getMesh(show_mesh_faces, ofxKFW2::Source::Depth::PointCloudOptions::ColorCamera);
	mesh.draw();

	if (!ofIsGLProgrammableRenderer()) {
		// TEMP: Until OF master fixes texture binding for old pipeline.
		kinect.getColorSource()->getTexture().unbind(2);
		kinect.getBodyIndexSource()->getTexture().unbind(1);
	}
	shader.end();
	// ----------------------------------------------------


	// --------------- Show 3D Skeleton -------------------
	if (show_skeletons) {

			kinect.getBodySource()->drawWorld();
	}

	ofPopMatrix();

	ofPushMatrix();
	// scale from M to MM to draw the rest of the 3D world
	ofScale(1000, 1000, 1000);
	draw_world();
	ofPopMatrix();

	// show the Body being streamed out
	if (closest_body != nullptr)
		draw_body(closest_body);

	ofDrawAxis(1500);
	cam.end();
	/////////////////////////////////////////////////////////


	// --------------- Show 3D Skeleton -------------------
	if (show_gui) {
		ofPushStyle();
		if (do_streaming)
			ofSetColor(ofColor::aqua, 40);
		else
			ofSetColor(100, 80);
		ofDrawRectangle(panel.getPosition().x-5, panel.getPosition().y - 5, panel.getWidth() + 10, panel.getHeight() + 15);
		ofPopStyle();

		panel.draw();

		ofSetColor(0);
		stringstream ss;
		ss << "Streaming is: " << ((do_streaming) ? "ON" : "OFF") << endl;
		ss << "Number of Tracked Bodies: " << bodies.size() << endl;
		ofDrawBitmapString(ss.str(), 10, ofGetHeight() - 15);

		ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), ofGetWidth() - 55, 15);


		// draw crouch visual debugger
		if (show_crouch) {

			ofPushStyle();
			float w = 200;
			float h = 100;
			float padding = 10;

			ofPushMatrix();
			ofTranslate(panel.getPosition().x, panel.getPosition().y + panel.getHeight() + padding);
			ofSetColor(120);
			ofFill();
			ofDrawRectangle(0, 0, w, h);
			ofSetColor(60);
			ofDrawRectangle(padding/2, padding / 2, w-padding, 25);
			ofSetColor(0);
			ofDrawRectangle(padding / 2, 25, w - padding, h - padding - 20);
			ofSetLineWidth(5);
			ofVec2f start = ofVec2f(2*padding, h / 2);
			ofVec2f end = ofVec2f(w-2*padding, h / 2);
			ofSetColor(120);
			ofDrawLine(start, end);
			ofSetColor(ofColor::orange);
			ofVec3f lerp = start.getInterpolated(end, crouch_scalar);
			ofDrawEllipse(lerp, 20, 20);
			ofSetColor(255);
			ofDrawBitmapString("Crouch Visualizer", padding, 2*padding);
			ofSetLineWidth(1);
			ofDrawLine(lerp.x, lerp.y + 4 * padding, lerp.x, lerp.y );
			ofDrawBitmapStringHighlight(ofToString(crouch_scalar), lerp.x, lerp.y + 4*padding);
			ofDrawBitmapString("0", start.x, start.y + 2*padding);
			ofDrawBitmapString("1", end.x, end.y + 2*padding);
			ofPopMatrix();
			
			ofPopStyle();

		}
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();

	panel.saveToFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	bool val = true;
	switch (key) {
	case 'g':
	case 'G':
		show_gui = !show_gui;

		if (show_gui) {
			ofSetFullscreen(false);
		}
		else
			ofSetFullscreen(true);
		break;
	case 'm':
	case 'M':
		listener_mirror(val);
		break;
	case 'a':
		listener_avoid(val);
		break;
	case 'i':
	case 'I':
		listener_idle(val);
		break;
	case 'o':
	case 'O':
		listener_other(val);
		break;
	default:
		handle_keypressed_cam(key);
	}
}

//--------------------------------------------------------------
void ofApp::setup_osc(){
	string ip_addr = "127.0.0.1";
	int port = 55555;

	// if we have incoming args, override the default ip address and port
	if (args.size() == 3) {
		cout << "Getting CMD input {" << args[1] << ":" << args[2] << "}" << endl;

		ip_addr = args[1];
		sscanf(args[2].c_str(), "%d", &port);
		
		port;
	}

	cout << "Setup OSC COMs at {" << ip_addr << ":" << ofToString(port) << "}" << endl;
	sender.setup(ip_addr, port);
}

//--------------------------------------------------------------
void ofApp::setup_sensor()
{
	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();
	kinect.initBodySource();
	kinect.initBodyIndexSource();
}

//--------------------------------------------------------------
void ofApp::update_sensor()
{
	kinect.update();
}

//--------------------------------------------------------------
void ofApp::setup_world()
{
	ofVec3f val = interaction_zone_offset;

	float i_zone_width = 2;
	float i_zone_depth = 2.5;
	float i_zone_height = 2;
	ofVec3f i_zone_centroid = ofVec3f(i_zone_width / 2 + val.x, 0 + val.y, i_zone_depth / 2 - 2 * .254 + val.z);

	interaction_zone.setGlobalPosition(i_zone_centroid);
	interaction_zone.setWidth(i_zone_width);
	interaction_zone.setHeight(i_zone_height);
	interaction_zone.setDepth(i_zone_depth);


	//	robot_bounds.setGlobalPosition((robot_bounds_min.get() + robot_bounds_max.get()) / 2);
//	robot_bounds.setWidth(robot_bounds_max.get().x - robot_bounds_min.get().x);		// X Dim
//	robot_bounds.setHeight(robot_bounds_max.get().y - robot_bounds_min.get().y);	// Y Dim
//	robot_bounds.setDepth(robot_bounds_max.get().z - robot_bounds_min.get().z);	

	// setup camera defaults
	cam.setFarClip(9999);
	cam.setNearClip(1);
	cam.setDistance(5000);
	bool flag = true;
	listener_show_perspective(flag);
}

//--------------------------------------------------------------
//void ofApp::setup_world()
//{
//	// mirror_plane width and height is in Meters
//	mirror_plane.setMode(OF_PRIMITIVE_TRIANGLES);
//	mirror_plane.addVertex(ofPoint(0, -mirror_plane_width / 2.,  mirror_plane_height / 2.));
//	mirror_plane.addVertex(ofPoint(0, -mirror_plane_width / 2., -mirror_plane_height / 2.));
//	mirror_plane.addVertex(ofPoint(0,  mirror_plane_width / 2., -mirror_plane_height / 2.));
//	mirror_plane.addVertex(ofPoint(0,  mirror_plane_width / 2.,  mirror_plane_height / 2.));
//
//	mirror_plane.addIndex(0);
//	mirror_plane.addIndex(1);
//	mirror_plane.addIndex(2);
//
//	mirror_plane.addIndex(2);
//	mirror_plane.addIndex(0);
//	mirror_plane.addIndex(3);
//
//	listener_mirror_plane_offset(ofPoint(mirror_plane_offset.get().x, mirror_plane_offset.get().y, mirror_plane_offset.get().z));
//
//	// the robot_bounds is in Meters
//	robot_bounds.setGlobalPosition((robot_bounds_min.get() + robot_bounds_max.get()) / 2);
//	robot_bounds.setWidth(robot_bounds_max.get().x - robot_bounds_min.get().x);		// X Dim
//	robot_bounds.setHeight(robot_bounds_max.get().y - robot_bounds_min.get().y);	// Y Dim
//	robot_bounds.setDepth(robot_bounds_max.get().z - robot_bounds_min.get().z);		// Z Dim
//
//	// setup the filtered points for left and right targets
//	fp_tgt_left.setup();
//	fp_tgt_right.setup();
//	// add the fp_tgts to the gui
//	//panel.add(fp_tgt_left.get_gui());
//	//panel.add(fp_tgt_right.get_gui());
//
//	// setup avoidance cylinders
//	arm_left.set(.050, .100, true);
//	arm_left.setResolutionRadius(25);
//	arm_right.set(.050, .100, true);
//	arm_right.setResolutionRadius(25);
//
//	// setup avoidance spheres
//	hand_sphere_left.set(.1, 10);
//	hand_sphere_right.set(.1, 10);
//
//	// setup camera defaults
//	cam.setFarClip(9999);
//	cam.setNearClip(1);
//	cam.setDistance(5000);
//	bool val = true;
//	listener_show_perspective(val);
//
//	// setup avoid zone inside the robot bounds listener
//	listener_robot_bounds(ofPoint(robot_bounds_max.get()));
//}

//--------------------------------------------------------------
void ofApp::draw_world()
{
	ofPushStyle();
	ofSetLineWidth(3);
	//ofSetColor(ofColor::magenta, 100);
	//mirror_plane.drawWireframe();

	// draw the floor
	ofPlanePrimitive floor;
	floor.set(7, 5);
	ofPushMatrix();
	ofTranslate(0, 0, -2 * .254);
	ofFill();
	ofSetColor(100, 100);
	floor.draw();
	ofPopMatrix();

	// draw the stage
	float table_w = 6 * .254;
	float table_h = 8 * .254;
	float table_d = 2 * .254;
	ofVec3f table_centroid = ofVec3f(-table_w/2, 0, -table_d/2);
	ofSetColor(100, 100);
	ofDrawBox(table_centroid, table_w, table_h, table_d);
	ofNoFill();
	ofSetColor(100);
	ofDrawBox(table_centroid, table_w, table_h, table_d);

	// draw the sensor
	ofPushMatrix();
	ofTranslate(sensor_offset);
	ofRotate(90);
	ofRotateX(90);
	ofRotateX(sensor_tilt);
	ofScale(.001, .001, .001);
	draw_frustum();
	ofPopMatrix();


	// draw the INTERACTION ZONE
	ofPlanePrimitive zone_inter;
	zone_inter.set(interaction_zone.getWidth(), interaction_zone.getHeight());
	ofPushMatrix();
	ofTranslate(interaction_zone.getGlobalPosition().x, interaction_zone.getGlobalPosition().y, -table_d );
	ofFill();
	ofSetColor(ofColor::deepSkyBlue, 80);
	zone_inter.draw();
	ofNoFill();
	ofSetLineWidth(3);
	ofSetColor(ofColor::deepSkyBlue);
	ofPopMatrix();
	ofPushMatrix();
	ofTranslate(interaction_zone.getGlobalPosition().x - interaction_zone.getWidth()/2, interaction_zone.getGlobalPosition().y - interaction_zone.getHeight()/2, -table_d);
	ofDrawRectangle(0, 0, interaction_zone.getWidth(), interaction_zone.getHeight());
	ofPopMatrix();
	ofSetLineWidth(1);
	interaction_zone.drawWireframe();


	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::setup_shader()
{
	if (ofIsGLProgrammableRenderer()) {
		shader.load("shaders_gl3/bodyIndex");
	}
	else {
		shader.load("shaders/bodyIndex");
	}
}

//--------------------------------------------------------------
void ofApp::update_bodies()
{
	// update the Bodies map
	auto _bodies = kinect.getBodySource()->getBodies();
	for (auto & _body : _bodies) {
		update_body(_body);
	}
}

//--------------------------------------------------------------
void ofApp::update_body(ofxKinectForWindows2::Data::Body _body)
{
	int id = _body.bodyId;

	if (_body.tracked) {

		// if this is a new body, add it to our map
		if (bodies.find(id) == bodies.end()) {
			bodies.insert(make_pair(id, new Body()));
			bodies.at(id)->InitAsDefaultInstance();
			bodies.at(id)->set_id(id);
		}

		// update the body with new kinect and interaction info
		Body* body = bodies.at(id);
		// ... I should also add a timestamp to the body proto here

		// set interaction mode
		if (idle)
			body->set_interaction_mode(Body_InteractionMode_FOLLOW);
		else if (mirror)
			body->set_interaction_mode(Body_InteractionMode_MIRROR);
		else if (avoid)
			body->set_interaction_mode(Body_InteractionMode_AVOID);
		body->set_state(Body::ACTIVE);

		// clear the old joints and hand
		body->clear_joints();
		body->clear_hands();


		// iterate through the kinect's joint map to update the body proto
		int i = 0;
		map<JointType, ofxKinectForWindows2::Data::Joint>::iterator it;
		for (it = _body.joints.begin(); it != _body.joints.end(); it++) {

			ofNode joint;
			joint.setGlobalPosition(it->second.getPosition());
			joint.setGlobalOrientation(it->second.getOrientation());

			if (use_world_coordinates) {
		
				ofVec3f pos;
				float _x = joint.getGlobalPosition().z;
				float _y = joint.getGlobalPosition().x;
				float _z = joint.getGlobalPosition().y;

				pos.set(_x, _y, _z);
				pos.rotate(sensor_tilt, ofVec3f(0, 1, 0));
				pos += sensor_offset;

				joint.setGlobalPosition(pos);


				// @FIXME the joint's orienation should also be rotated :(
				//joint.setGlobalOrientation(....);
			}
			else {
				joint.setGlobalPosition(joint.getGlobalPosition() + sensor_offset);
			}

			// create new Proto Joint
			Pose* pose = new Pose();
			pose->InitAsDefaultInstance();
			pose->mutable_pos()->set_x(joint.getGlobalPosition().x);
			pose->mutable_pos()->set_y(joint.getGlobalPosition().y);
			pose->mutable_pos()->set_z(joint.getGlobalPosition().z);
			pose->mutable_orient()->set_x(joint.getGlobalOrientation().x());
			pose->mutable_orient()->set_y(joint.getGlobalOrientation().y());
			pose->mutable_orient()->set_z(joint.getGlobalOrientation().z());
			pose->mutable_orient()->set_w(joint.getGlobalOrientation().w());

			srl::body::Joint* proto_joint = new srl::body::Joint();
			proto_joint->InitAsDefaultInstance();
			proto_joint->mutable_pose()->Swap(pose);
			proto_joint->set_name(joint_names[i]);
			proto_joint->set_id(i);

			// add the proto joint to the proto body
			body->mutable_joints()->AddAllocated(proto_joint);

			// create new Proto Hands ... not the most efficient :(
			if (it->first == JointType::JointType_HandLeft || it->first == JointType::JointType_HandRight) {
				Hand* hand = new Hand();
				hand->InitAsDefaultInstance();
				hand->set_name(joint_names[i]);

				if (it->first == JointType::JointType_HandLeft) {
					switch (_body.leftHandState)
					{
					case HandState_NotTracked:
					case HandState_Unknown:
						hand->set_state(Hand_HandState_UNKNOWN);
						break;
					case HandState_Open:
						hand->set_state(Hand_HandState_OPEN);
						break;
					case HandState_Closed:
						hand->set_state(Hand_HandState_CLOSED);
						break;
					default:
						break;
					}
				}
				else {
					switch (_body.rightHandState)
					{
					case HandState_NotTracked:
					case HandState_Unknown:
						hand->set_state(Hand_HandState_UNKNOWN);
						break;
					case HandState_Open:
						hand->set_state(Hand_HandState_OPEN);
						break;
					case HandState_Closed:
						hand->set_state(Hand_HandState_CLOSED);
						break;
					default:
						break;
					}
				}
				body->mutable_hands()->AddAllocated(hand);
			}

			i++;
		}
	}
	else {
		//cout << "body {" << ofToString(bod.bodyId) << "} is not being tracked" << endl;


		// check if the body ID is already in our map and mark it DEAD
		// if this is a new body, add it to our map
		// @FIXME ... not working for receiver
		if (bodies.find(id) != bodies.end()) {
			// if the body was previously marked DEAD, remove from map
			if (bodies.at(id)->state() == Body::DEAD) {
				cout << "REMOVING BODY {" << id << "} from the map." << endl;
				bodies.erase(id);
			}
			// otherwise marked the body as DEAD
			else {
				cout << "MARKING BODY {" << id << "} as DEAD." << endl;
				bodies.at(id)->set_state(Body::DEAD);
			}
		}
	}

}


//--------------------------------------------------------------
void ofApp::draw_body(Body* _body) {


	ofPushStyle();
	ofNoFill();

	ofSetColor(ofColor::white);

	// draw body joints
	draw_joints(_body);

	// draw body skeleton
	ofPushMatrix();
	ofScale(scalar, scalar, scalar);
	draw_skeleton(_body);
	ofPopMatrix();

	ofPopStyle();

}

//--------------------------------------------------------------
void ofApp::draw_joints(Body* _body) {
	for (int i = 0; i<_body->joints_size(); i++) {

		if (_body->mutable_joints(i)->has_pose()) {
			ofNode pose = toOf(_body->mutable_joints(i)->mutable_pose());

			// ignore the hand joints
			if (_body->mutable_joints(i)->name() == "HAND_RIGHT" || _body->mutable_joints(i)->name() == "HAND_TIP_RIGHT" || _body->mutable_joints(i)->name() == "THUMB_RIGHT" || _body->mutable_joints(i)->name() == "HAND_LEFT" || _body->mutable_joints(i)->name() == "HAND_TIP_LEFT" || _body->mutable_joints(i)->name() == "THUMB_LEFT") {
				// ... skip drawing the hands
			}
			else {
				ofNode temp;
				temp.setGlobalPosition(pose.getGlobalPosition() * scalar);
				temp.draw();
				ofDrawBox(temp.getGlobalPosition(), 50);
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw_skeleton(Body* _body) {

	if (_body->mutable_joints()->size() > 0) {

		// draw spine
		ofNode bone_a, bone_b;
		bone_a = toOf(_body->mutable_joints(SPINE_BASE)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(SPINE_MID)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(SPINE_MID)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(NECK)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(NECK)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(HEAD)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());

		// draw left arm
		bone_a = toOf(_body->mutable_joints(SPINE_SHOULDER)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(SHOULDER_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(SHOULDER_LEFT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(ELBOW_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(ELBOW_LEFT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(WRIST_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(WRIST_LEFT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(HAND_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());

		// draw right arm
		bone_a = toOf(_body->mutable_joints(SPINE_SHOULDER)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(SHOULDER_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(SHOULDER_RIGHT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(ELBOW_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(ELBOW_RIGHT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(WRIST_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(WRIST_RIGHT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(HAND_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());

		// draw left leg
		bone_a = toOf(_body->mutable_joints(SPINE_BASE)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(HIP_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(HIP_LEFT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(KNEE_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(KNEE_LEFT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(ANKLE_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(ANKLE_LEFT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(FOOT_LEFT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());

		// draw right leg
		bone_a = toOf(_body->mutable_joints(SPINE_BASE)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(HIP_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(HIP_RIGHT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(KNEE_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(KNEE_RIGHT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(ANKLE_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
		bone_a = toOf(_body->mutable_joints(ANKLE_RIGHT)->mutable_pose());
		bone_b = toOf(_body->mutable_joints(FOOT_RIGHT)->mutable_pose());
		ofDrawLine(bone_a.getGlobalPosition(), bone_b.getGlobalPosition());
	}

}

void ofApp::update_closest_body()
{
	// check that we actually have bodies in the map
	if (bodies.size() == 0) {
		ofLogError() << "THERE ARE CURRENTLY NO BODIES BEING STORED," << endl;
		return;
	}

	map<int, Body*>::iterator it;
	int i = 0;
	int closest_id = -1;
	float closest_dist = FLT_MAX;
	for (it = bodies.begin(); it != bodies.end(); it++) {

		if (bodies.size() == 1){
			closest_body = it->second;
			return;
		}
		else {
			//// FIXME! should be closest, not oldest
			//if (i == 0)
			//	closest_body = it->second;

			// the CLOSEST BODY is whichever's hands are closest to the origin

			Body * b = it->second;
			ofNode left = toOf(b->mutable_joints(HAND_LEFT)->mutable_pose());
			ofNode right = toOf(b->mutable_joints(HAND_RIGHT)->mutable_pose());

			// check if the left and/or right hands are 
			float dist_sq = left.getGlobalPosition().distanceSquared(ofVec3f());
			if (dist_sq < closest_dist) {
				closest_dist = dist_sq;
				closest_id = it->first;
			}
			dist_sq = right.getGlobalPosition().distanceSquared(ofVec3f());
			if (dist_sq < closest_dist) {
				closest_dist = dist_sq;
				closest_id = it->first;
			}
		}
		i++;
	}
	// set the closest body, if we have a valid key
	if (closest_id != -1) {
		closest_body = bodies.at(closest_id);
	}
	else {
		ofLogError() << "WTF?! why am I not getting the closest body?" << endl;
		closest_body = &default_body;
	}

}

void ofApp::generate_robot_targets()
{
	// find the closest skeleton to the robot

	ofNode hand_left, hand_right, elbow_left, elbow_right;
	

	bool update_rob_targets = false;
	if (idle) {

	}
	else if (mirror) {
		//// The user's left hand moves the robot's right hand,
		//// and the user's right hand moves the robot's left hand

		//ofVec3f left, right;
		//ofNode shoulder_right, shoulder_left;
		//
		//// the robot's left hand is the Person's right hand
		//hand_left = toOf(closest_body->mutable_joints(HAND_RIGHT)->mutable_pose());
		//hand_right = toOf(closest_body->mutable_joints(HAND_LEFT)->mutable_pose());

		//if (isInside(hand_right.getGlobalPosition(), avoid_zone)) {
		//	cout << " The Person's LEFT HAND is inside the AVOID ZONE" << endl;
		//	// make the robot's end-effector move directly to that point
		//	right.set(hand_right.getGlobalPosition());
		//}
		//// exaggerate the robot's movement to better match the person's pose
		//else {
		//	// get the hand pose relative to the shoulder
		//	shoulder_right = toOf(closest_body->mutable_joints(SHOULDER_LEFT)->mutable_pose());
		//	ofVec3f shoulder_to_hand_right = shoulder_right.getGlobalPosition() - hand_right.getGlobalPosition();
		//	// reflect for the robot's coords
		//	shoulder_to_hand_right.y *= -1;
		//	shoulder_to_hand_right.z *= -1;
		//	// move from origin to robot's right shoulder
		//	ofVec3f robot_shoulder_right = ofVec3f(.25, -.10, .45);
		//	// add a little extra distance on the Y-Axis to exaggerate the robot's reach
		//	float diff = shoulder_right.getGlobalPosition().y - hand_right.getGlobalPosition().y;
		//	float scalar = ofMap(diff, -.2, 1, 0, 1); // don't clamp; let the values go +/-
		//	robot_shoulder_right.y -= scalar;
		//	// add the offset and set the raw target position
		//	shoulder_to_hand_right += robot_shoulder_right;
		//	right.set(shoulder_to_hand_right);
		//}
		//// check if the hands are inside the "avoid" zone
		//if (isInside(hand_left.getGlobalPosition(), avoid_zone)) {
		//	cout << "\tThe Person's RIGHT HAND is inside the AVOID ZONE" << endl;
		//	// make the robot's end-effector move directly to that point
		//	left.set(hand_left.getGlobalPosition());
		//}
		//else {
		//	shoulder_left = toOf(closest_body->mutable_joints(SHOULDER_RIGHT)->mutable_pose());
		//	ofVec3f shoulder_to_hand_left = shoulder_left.getGlobalPosition() - hand_left.getGlobalPosition();
		//	// reflect for the robot's coords
		//	shoulder_to_hand_left.y *= -1;
		//	shoulder_to_hand_left.z *= -1;
		//	// move from origin to robot's left shoulder
		//	ofVec3f robot_shoulder_left = ofVec3f(.25, .10, .45);
		//	// add a little extra distance on the Y-Axis to exaggerate the robot's reach
		//	float diff = hand_left.getGlobalPosition().y - shoulder_left.getGlobalPosition().y;
		//	float scalar = ofMap(diff, -.2, 1, 0, 1); // don't clamp; let the values go +/-
		//	robot_shoulder_left.y += scalar;
		//	// add the offset and set the raw target position
		//	shoulder_to_hand_left += robot_shoulder_left;
		//	left.set(shoulder_to_hand_left);
		//}
		// 

		//// clamp the z value to stay within the robot_bounds (Z only)
		//left.z = ofClamp(left.z, robot_bounds_min.get().z, robot_bounds_max.get().z);
		//right.z = ofClamp(right.z, robot_bounds_min.get().z, robot_bounds_max.get().z);

		//// update the filtered point with the raw left and right values		
		//fp_tgt_left.update(left);
		//fp_tgt_right.update(right);

		//ofVec3f filtered_right = fp_tgt_right.get_filtered();
		//ofVec3f filtered_left = fp_tgt_left.get_filtered();
		//
		//// update the targets with filtered left and right poses
		//target_left.setGlobalPosition(filtered_left);
		//target_right.setGlobalPosition(filtered_right);
		//target_left.setGlobalOrientation(ofQuaternion(0, 0, 0, 1));
		//target_right.setGlobalOrientation(ofQuaternion(0, 0, 0, 1));


		//update_rob_targets = true;

	}
	else if (avoid) {

		//ofVec3f left, right;

		//// create avoidance spheres at the end of each hand
		//left = toOf(closest_body->mutable_joints(HAND_LEFT)->mutable_pose()).getGlobalPosition();
		//right = toOf(closest_body->mutable_joints(HAND_RIGHT)->mutable_pose()).getGlobalPosition();

		//// update the filtered point with the left and right values
		//fp_tgt_left.update(left);
		//fp_tgt_right.update(right);

		//// update spheres (for rendering purposes only) 
		//hand_sphere_left.setGlobalPosition(fp_tgt_left.get_filtered());
		//hand_sphere_right.setGlobalPosition(fp_tgt_right.get_filtered());

		//// update the targets with filtered left and right poses
		//target_left.setGlobalPosition(fp_tgt_left.get_filtered().x, fp_tgt_left.get_filtered().y, fp_tgt_left.get_filtered().z);
		//target_left.setGlobalOrientation(ofQuaternion(0,0,0,1));
		//target_right.setGlobalPosition(fp_tgt_right.get_filtered().x, fp_tgt_right.get_filtered().y, fp_tgt_right.get_filtered().z);
		//target_right.setGlobalOrientation(ofQuaternion(0, 0, 0, 1));

		//update_rob_targets = true;
		
	}

	// Create the Target Poses and add to the closest body
	// ... currently default orientation data
	if (update_rob_targets) {
		update_robot_targets();
	}

}

//-------------------------------------------------------------
void ofApp::update_robot_targets()
{
	//// Left
	//Pose* pose_left = new Pose();
	//pose_left->InitAsDefaultInstance();
	//pose_left->mutable_pos()->set_x(target_left.getGlobalPosition().x);
	//pose_left->mutable_pos()->set_y(target_left.getGlobalPosition().y);
	//pose_left->mutable_pos()->set_z(target_left.getGlobalPosition().z);
	//pose_left->mutable_orient()->set_x(target_left.getGlobalOrientation().x());
	//pose_left->mutable_orient()->set_y(target_left.getGlobalOrientation().y());
	//pose_left->mutable_orient()->set_z(target_left.getGlobalOrientation().z());
	//pose_left->mutable_orient()->set_w(target_left.getGlobalOrientation().w());
	//closest_body->mutable_targets()->mutable_left()->Swap(pose_left);

	//// Right
	//Pose* pose_right = new Pose();
	//pose_right->InitAsDefaultInstance();
	//pose_right->mutable_pos()->set_x(target_right.getGlobalPosition().x);
	//pose_right->mutable_pos()->set_y(target_right.getGlobalPosition().y);
	//pose_right->mutable_pos()->set_z(target_right.getGlobalPosition().z);
	//pose_right->mutable_orient()->set_x(target_right.getGlobalOrientation().x());
	//pose_right->mutable_orient()->set_y(target_right.getGlobalOrientation().y());
	//pose_right->mutable_orient()->set_z(target_right.getGlobalOrientation().z());
	//pose_right->mutable_orient()->set_w(target_right.getGlobalOrientation().w());
	//closest_body->mutable_targets()->mutable_right()->Swap(pose_right);
}

//-------------------------------------------------------------
void ofApp::setup_client()
{
	// set default values
	string ip_addr = "127.0.0.1";
	int port = 55555;

	// if we have incoming args, override the default values
	if (args.size() == 3) {
		cout << "Getting CMD input {"<< args[1] << ":" << args[2] << "}" << endl;
		
		ip_addr = args[1];
		sscanf(args[2].c_str(), "%d", &port);
		// TEMP: increment the port so it's not the same as OSC's
		port++;
	}

	client.setup(ip_addr, port);

	default_body.InitAsDefaultInstance();
	default_body.set_interaction_mode(Body_InteractionMode_FOLLOW);
}

//-------------------------------------------------------------
void ofApp::setup_gui()
{
	ofSetFullscreen(true);

	params.setName("Debug_View");
	params.add(show_skeletons.set("Show_Skeletons", true));
	params.add(show_mesh_faces.set("Show_Mesh", false));
	params.add(use_world_coordinates.set("Use_World_Coords", true));
	params.add(do_streaming.set("Stream_Bodies", true));

	params_sensor.setName("Sensor_Params");
	params_sensor.add(sensor_offset.set("Offset", ofVec3f(-.30, -.30, -.65), ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1)));
	params_sensor.add(sensor_tilt.set("Tilt", -25, -90, 90));

	params_interaction.setName("Interaction_Params");
	params_interaction.add(idle.set("Idle", true));
	params_interaction.add(mirror.set("Mirror", false));
	params_interaction.add(avoid.set("Avoid", false));
	params_interaction.add(other.set("Other", false));
	params_interaction.add(interaction_zone_offset.set("Interaction_Zone_Offset", ofVec3f(), ofVec3f(0,-2,0), ofVec3f(2,2,0)));
	params_interaction.add(crouch_dist_min.set("Crouch_Dist_Min", 0, 0, 1));
	params_interaction.add(crouch_dist_max.set("Crouch_Dist_Max", 1, 0, 2));
	params_interaction.add(show_crouch.set("Show_Crouch",true));
	//params_interaction.add(mirror_plane_offset.set("Plane_Offset", ofVec3f(.6,0,0), ofVec3f(-1, -1, -1), ofVec3f(1, 1, 1)));
	//params_interaction.add(robot_bounds_min.set("Robot_Bounds_Min", ofVec3f(-.5, -.8, 0), ofVec3f(0, -1, -1), ofVec3f(1, 1, 1)));
	//params_interaction.add(robot_bounds_max.set("Robot_Bounds_Max", ofVec3f(1, .8, .9), ofVec3f(0, -1, -1), ofVec3f(1, 1, 2.5)));

	idle.addListener(this, &ofApp::listener_idle);
	mirror.addListener(this, &ofApp::listener_mirror);
	avoid.addListener(this, &ofApp::listener_avoid);
	other.addListener(this, &ofApp::listener_other);
	interaction_zone_offset.addListener(this, &ofApp::listener_interaction_zone_offset);
	//mirror_plane_offset.addListener(this, &ofApp::listener_mirror_plane_offset);
	//robot_bounds_min.addListener(this, &ofApp::listener_robot_bounds);
	//robot_bounds_max.addListener(this, &ofApp::listener_robot_bounds);



	panel.setup(params);
	panel.add(params_sensor);
	panel.add(params_interaction);
	panel.setPosition(10, 10);

	if (load_params_from_file)
		panel.loadFromFile("settings.xml");

}

//-------------------------------------------------------------
void ofApp::listener_idle(bool &val)
{
	if (val) {
		idle.set(true);
		mirror.set(false);
		avoid.set(false);
		other.set(false);
	}
}

void ofApp::listener_mirror(bool &val)
{
	if (val) {
		mirror.set(true);
		idle.set(false);
		avoid.set(false);
		other.set(false);
	}
}

void ofApp::listener_avoid(bool &val)
{
	if (val) {
		idle.set(false);
		mirror.set(false);
		avoid.set(true);
		other.set(false);
	}
}

void ofApp::listener_other(bool &val)
{
	if (val) {
		idle.set(false);
		mirror.set(false);
		avoid.set(false);
		other.set(true);
	}
}

void ofApp::listener_interaction_zone_offset(ofVec3f & val) {

	float i_zone_width = 2;
	float i_zone_depth = 2.5;
	float i_zone_height = 2;
	ofVec3f i_zone_centroid = ofVec3f(i_zone_width / 2 + val.x, 0 + val.y, i_zone_depth / 2 - 2 * .254 + val.z);

	interaction_zone.setGlobalPosition(i_zone_centroid);
	interaction_zone.setWidth(i_zone_width);
	interaction_zone.setHeight(i_zone_height);
	interaction_zone.setDepth(i_zone_depth);

}

//void ofApp::listener_mirror_plane_offset(ofVec3f & val)
//{
//	mirror_plane.setVertex(0, ofVec3f(0 + val.x, -mirror_plane_width / 2. + val.y,  mirror_plane_height / 2. + val.z));
//	mirror_plane.setVertex(1, ofVec3f(0 + val.x, -mirror_plane_width / 2. + val.y, -mirror_plane_height / 2. + val.z));
//	mirror_plane.setVertex(2, ofVec3f(0 + val.x,  mirror_plane_width / 2. + val.y, -mirror_plane_height / 2. + val.z));
//	mirror_plane.setVertex(3, ofVec3f(0 + val.x,  mirror_plane_width / 2. + val.y,  mirror_plane_height / 2. + val.z));
//}
//void ofApp::listener_robot_bounds(ofVec3f & val)
//{
//	robot_bounds.setGlobalPosition((robot_bounds_min.get() + robot_bounds_max.get()) / 2);
//	robot_bounds.setWidth(robot_bounds_max.get().x - robot_bounds_min.get().x);		// X Dim
//	robot_bounds.setHeight(robot_bounds_max.get().y - robot_bounds_min.get().y);	// Y Dim
//	robot_bounds.setDepth(robot_bounds_max.get().z - robot_bounds_min.get().z);		// Z Dim
//
//	avoid_zone.setGlobalPosition(robot_bounds.getGlobalPosition());
//	avoid_zone.setWidth(robot_bounds.getWidth());		// X Dim
//	avoid_zone.setHeight(robot_bounds.getHeight());	// Y Dim
//	avoid_zone.setDepth(3);		// Z Dim
//
//	float mirror_zone_min_y = robot_bounds.getHeight();
//	mirror_zone.setWidth(1.5);
//	mirror_zone.setHeight(robot_bounds.getHeight() + .25);
//	mirror_zone.setDepth(3);
//	mirror_zone.setGlobalPosition(avoid_zone.getWidth() + .25, 0, robot_bounds.getGlobalPosition().z);
//
//
//}
//-------------------------------------------------------------

//-------------------------------------------------------------
ofNode ofApp::toOf(Pose * pose)
{
	double x = pose->mutable_pos()->x();
	double y = pose->mutable_pos()->y();
	double z = pose->mutable_pos()->z();

	double qx = pose->mutable_orient()->x();
	double qy = pose->mutable_orient()->y();
	double qz = pose->mutable_orient()->z();
	double qw = pose->mutable_orient()->w();

	ofNode _pose;
	_pose.setGlobalPosition(x, y, z);
	_pose.setGlobalOrientation(ofQuaternion(qx, qy, qz, qw));
	return _pose;
}

bool ofApp::isInside(ofVec3f pt, ofBoxPrimitive aabb)
{
	float aabb_min_x, aabb_max_x;
	float aabb_min_y, aabb_max_y;
	float aabb_min_z, aabb_max_z;

	ofVec3f centroid = aabb.getGlobalPosition();

	aabb_min_x = centroid.x - (aabb.getWidth() / 2);
	aabb_max_x = centroid.x + (aabb.getWidth() / 2);
	aabb_min_y = centroid.y - (aabb.getHeight() / 2);
	aabb_max_y = centroid.y + (aabb.getHeight() / 2);
	aabb_min_z = centroid.z - (aabb.getDepth() / 2);
	aabb_max_z = centroid.z + (aabb.getDepth() / 2);
	
	if ((pt.x > aabb_min_x && pt.x < aabb_max_x) &&
		(pt.y > aabb_min_y && pt.y < aabb_max_y) &&
		(pt.z > aabb_min_z && pt.z < aabb_max_z))
		return true;

	return false;
}

//-------------------------------------------------------------
void ofApp::handle_keypressed_cam(int key)
{
	bool val = true;
	switch (key)
	{
	case '1':
		listener_show_top(val);
		break;
	case '2':
		listener_show_front(val);
		break;
	case '3':
		listener_show_side(val);
		break;
	case '4':
		listener_show_perspective(val);
		break;
	default:
		break;
	}
}

// --------------------------------------------------------------
void ofApp::listener_show_top(bool & val)
{
	if (val) {

		int x = 0;
		int y = (800) / 2;
		int z = 5000;

		ofVec3f pos = ofVec3f(x, y, z);
		ofVec3f tgt = ofVec3f(pos.x, pos.y, 0);
		cam.setGlobalPosition(pos);
		cam.setTarget(tgt);
		cam.lookAt(tgt, ofVec3f(1, 0, 0));

		//show_front = false;
		//show_side = false;
		//show_perspective = false;
	}
}

//--------------------------------------------------------------
void ofApp::listener_show_front(bool & val)
{
	if (val) {

		int x = 5000;
		int y = 0;
		int z = 600;

		ofVec3f pos = ofVec3f(x, y, z);
		ofVec3f tgt = ofVec3f(0, pos.y, pos.z);
		cam.setGlobalPosition(pos);
		cam.setTarget(tgt);
		cam.lookAt(tgt, ofVec3f(0, 0, 1));

		//show_top = false;
		//show_side = false;
		//show_perspective = false;
	}
}

//--------------------------------------------------------------
void ofApp::listener_show_side(bool & val)
{
	if (val) {

		int x = 1500;
		int y = -6000;
		int z = 600;

		ofVec3f pos = ofVec3f(x, y, z);
		ofVec3f tgt = ofVec3f(pos.x, 0, pos.z);
		cam.setGlobalPosition(pos);
		cam.setTarget(tgt);
		cam.lookAt(tgt, ofVec3f(0, 0, 1));

		//show_top = false;
		//show_front = false;
		//show_perspective = false;
	}
}

//--------------------------------------------------------------
void ofApp::listener_show_perspective(bool & val)
{
	if (val) {

		int x = -2000;
		int y = -2000;
		int z = 2000;


		ofVec3f pos = ofVec3f(x, y, z);
		ofVec3f tgt = ofVec3f(500, 0, 0);
		cam.setGlobalPosition(pos);
		cam.setTarget(tgt);
		cam.lookAt(tgt, ofVec3f(0, 0, 1));
		cam.setGlobalPosition(pos);

		//show_top = false;
		//show_front = false;
		//show_side = false;
	}
}

//--------------------------------------------------------------
void ofApp::draw_frustum() {

	ofPushStyle();
	ofEnableDepthTest();
	ofNoFill();

	ofSetLineWidth(2);

	// draw kinect
	ofDrawRectangle(ofPoint(-270 / 2, -40 / 2, 0), 270, 40);
	ofDrawRectangle(ofPoint(-220 / 2, -40 / 2, -50), 220, 40);
	ofDrawLine(270 * .5f, 40 * .5f, 0.0f, 220 * .5f, 40 * .5f, -50.0f);
	ofDrawLine(-270 * .5f, 40 * .5f, 0.0f, -220 * .5f, 40 * .5f, -50.0f);
	ofDrawLine(-270 * .5f, -40 * .5f, 0.0f, -220 * .5f, -40 * .5f, -50.0f);
	ofDrawLine(270 * .5f, -40 * .5f, 0.0f, 220 * .5f, -40 * .5f, -50.0f);

	ofSetLineWidth(1);

	// draw FOV lines
	float distDepth = 4000;// farClip; // should this be the fbo value? ... change to near/farClip
	float FovH = 1.0144686707507438;
	float FovV = 0.78980943449644714;

	float valueH = distDepth * tanf(FovH * .5f);
	float valueV = distDepth * tanf(FovV * .5f);

	ofDrawLine(0, 0, 0, valueH, valueV, distDepth);
	ofDrawLine(0, 0, 0, -valueH, valueV, distDepth);
	ofDrawLine(0, 0, 0, valueH, -valueV, distDepth);
	ofDrawLine(0, 0, 0, -valueH, -valueV, distDepth);

	// draw FarClip
	ofDrawRectangle(ofPoint(-valueH, -valueV, distDepth), 2 * valueH, 2 * valueV);

	distDepth = 800;// nearClip;
	valueH = distDepth * tanf(FovH * .5f);
	valueV = distDepth * tanf(FovV * .5f);

	// draw NearClip
	ofDrawRectangle(ofPoint(-valueH, -valueV, distDepth), 2 * valueH, 2 * valueV);

	ofDisableDepthTest();
	ofPopStyle();

}
