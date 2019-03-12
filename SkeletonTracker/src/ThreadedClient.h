#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"

//#include "egm.pb.h"
//#include "test.pb.h"
#include "body.pb.h"




using namespace srl::body;

class ThreadedClient : ofThread {
public:
	ThreadedClient() {}

	ofxUDPManager udpConnection;
    
    Body* body_message;
    bool has_new_message = false;
    
	int PORT = 11310;
    
    void setup(){
        
        udpConnection.Create();
        udpConnection.SetNonBlocking(false);
		udpConnection.Connect("127.0.0.1", PORT);
		//udpConnection.Connect("192.168.125.100", PORT);
        isInitialized = true;
//        udpConnection.Connect("192.168.125.100", portNumber);
//        //udpConnection.Bind(portNumber);
        body_message = new Body();
        body_message->InitAsDefaultInstance();
        
        start();
    }
    
    void setup(string ip_addr, int _port, bool set_non_blocking=false){

		cout << "Command Line Input for {IP_ADDR:PORT}: {" << ofToString(ip_addr) << ":" << ofToString(_port) << "}" << endl;

		PORT = _port;

        udpConnection.Create();
        udpConnection.SetNonBlocking(set_non_blocking);
        udpConnection.Connect(ip_addr.c_str(), PORT);
        isInitialized = true;

		body_message = new Body();
		body_message->InitAsDefaultInstance();

		start();
    }

    void update_message(Body* body){
        body_message = body;
        has_new_message = true;
    }
	

//    void setup(ofVec3f* _pos, ofQuaternion* _quat) {
//        pos_0 = _pos;
//        quat_0 = _quat;
//
//        udpConnection.Create();
//        udpConnection.SetNonBlocking(false);
//        udpConnection.Connect("192.168.125.100", portNumber);
//        //udpConnection.Bind(portNumber);
//
//        dummy_actual.InitAsDefaultInstance();
//        dummy_desired.InitAsDefaultInstance();
//        dummy_desired.mutable_header()->set_seqno(0);
//        dummy_desired.mutable_header()->set_tm(0);
//        dummy_desired.mutable_header()->set_mtype(EgmHeader_MessageType::EgmHeader_MessageType_MSGTYPE_COMMAND);
//
//        set_pose(pos_0, quat_0);
//    }

	bool connectToServer() {
		// send ping to server
		string messageBuffer = "ping!";
		cout << "sending <ping>" << endl;
		auto n = udpConnection.Send(messageBuffer.c_str(), messageBuffer.length());
		if (n < 0) {
			ofLogError() << "error while sending message" << endl;
			return false;
		}
		// wait for server reply
		cout << "waiting for server reply" << endl;
		char udpMessage[14000];
		n = udpConnection.Receive(udpMessage, 14000);
		if (n < 0) {
			ofLogError() << "error while receiving message" << endl;
			return false;
		}
		else {
			cout << "Connection to Sever is setup on PORT " << PORT << " and is ready for robots." << endl;
			string message = udpMessage;
			cout << "\tFirst Reply from Server: " << message << endl << endl;
		}
		// if we've made it here, we are good to go!
		return true;
	};


	//int msgCounter = 0;
    
    void SendMessage(Body* body){
        
        string messageBuffer;
        body->SerializeToString(&messageBuffer);
        //cout << "sending body " << ofToString(body->id()) << " to server ... buffer size {" << messageBuffer.length() << "}" << endl;       
		if (messageBuffer.length() > 2) { // if it's not the default body, broadcast out
			auto n = udpConnection.Send(messageBuffer.c_str(), messageBuffer.length());
			if (n < 0) {
				ofLogError() << "error while sending message" << endl;
			}
		}
		
    }
    
    
    // Not using ... just send out messages
    void ReceiveMessage(){
        char udpMessage[14000];
        auto n = udpConnection.Receive(udpMessage, 14000);
        cout << "\treceiving n bytes: " << n << endl;
        if (n > 0) {
   
            // get the proto message
            Body *pMessage = new Body();
            pMessage->ParseFromArray(udpMessage, udpConnection.GetReceiveBufferSize());    // deserializes inbound message
            
            stringstream ss;
            ss << "Message:\n";
            // TODO: do something with message
            cout << ss.str() << endl;
            
        }
        else {
            if (n == 0) {
                ofLogNotice() << "Receiving an EMPTY MESSAGE " << endl;
            }
            else {
                ofLogError() << "{n = " << n << "} error while receiving message from SERVER " << endl;
            }
        }
    }
/*
	//--------------------------------------------------------------
	// Waits to receive a message from the Server.
	// Saves the incoming ROBOT message into the correct robot_state
	// NOTE: The incoming ROBOT message has it ID encoded into it EgmTestSignals
	// NOTE: We should only be receiving VALID or EMPTY (n=0) messages from the Server.
	void ReceiveActual() {//Robot* robot) {

						  // we know we are getting a message back from robot->getState().id
		char udpMessage[14000];
		auto n = udpConnection.Receive(udpMessage, 14000);
		cout << "\treceiving n bytes: " << n << endl;
		if (n > 0) {
			//msgCounter++;
			//cout << endl << "INCOMING ROBOT MESSAGE # " << msgCounter << endl;
			// get the robot message
			EgmRobot *pMessage = new EgmRobot();
			pMessage->ParseFromArray(udpMessage, udpConnection.GetReceiveBufferSize());	// deserializes inbound message

			stringstream ss;
			ss << "Robot State:\n";

			ss << "\tActual State:\n";
			if (pMessage->has_header() && pMessage->header().has_seqno() && pMessage->header().has_tm() && pMessage->header().has_mtype()) {
				ss << "\t\t" << "SeqNo=" << pMessage->header().seqno() << " Tm=" << pMessage->header().tm() << " MsgType=" << pMessage->header().mtype() << "\n";

				// Motor State
				if (pMessage->has_motorstate()) { ss << "\t\tMotorstate: " << pMessage->motorstate().MotorStateType_Name(pMessage->motorstate().state()) << "\n"; }

				// Rapid Execution State
				//if (pMessage->has_rapidexecstate()) { ss << "\t\tMotorstate: " << pMessage->rapidexecstate().RapidCtrlExecStateType_Name(pMessage->rapidexecstate().state()) << "\n"; }

				if (pMessage->has_feedback()) {
					ss << "\t\tPose:\t";
					// Pose Data
					if (pMessage->feedback().has_cartesian() && pMessage->feedback().cartesian().has_pos() && pMessage->feedback().cartesian().has_orient()) {
						ss << "[[" << pMessage->feedback().cartesian().pos().x() << ", " << pMessage->feedback().cartesian().pos().y() << ", " << pMessage->feedback().cartesian().pos().z();
						ss << "], [" << pMessage->feedback().cartesian().orient().u0() << ", " << pMessage->feedback().cartesian().orient().u1() << ", ";
						ss << pMessage->feedback().cartesian().orient().u2() << ", " << pMessage->feedback().cartesian().orient().u3() << "]] \n";
					}
					// Joint Data
					if (pMessage->feedback().has_joints()) {
						ss << "\t\tJoints:\t[ ";
						for (auto &joint : pMessage->feedback().joints().joints())
							ss << joint << " ";
						ss << "]\n ";
					}
				}
				else { ss << "\tNo Actual State Data\n"; }

					ss << "\tNext Planned State:\n";
					if (pMessage->has_planned()) {
						// Planned Pose
						ss << "\t\tPlanned Pose:\t";
						if (pMessage->planned().has_cartesian() && pMessage->planned().cartesian().has_pos() && pMessage->feedback().cartesian().has_orient()) {
							ss << "[[" << pMessage->planned().cartesian().pos().x() << ", " << pMessage->planned().cartesian().pos().y() << ", " << pMessage->planned().cartesian().pos().z();
							ss << "], [" << pMessage->planned().cartesian().orient().u0() << ", " << pMessage->planned().cartesian().orient().u1() << ", ";
							ss << pMessage->planned().cartesian().orient().u2() << ", " << pMessage->planned().cartesian().orient().u3() << "]] \n";
						}
						// Joint Data
						if (pMessage->planned().has_joints()) {
							ss << "\t\tPlanned Joints:\t[ ";
							for (auto &joint : pMessage->planned().joints().joints())
								ss << joint << " ";
							ss << " ]\n ";
						}
					}
					else { ss << "\tNo Planned State Data\n"; }
				
			}
			else { ss << "\t\t... No Actual State info ...\n"; }

			cout << ss.str() << endl;

			
																						// we can't gaurentee which robot message comes back from the server, 
																						// so use the desired robot id encoded into the EgmRobot Message
			//int encoded_robot_id = RobotState::getID(pMessage);

			//// if we have a valid robot id
			//if (encoded_robot_id != -1) {
			//	auto robot_list = *robots;
			//	auto r = robot_list[encoded_robot_id];

			//	// if this is the first robot message
			//	if (!r->isInitialized()) {

			//		//cout << "THIS IS THE FIRST ROBOT MESSAGE FOR robot " << r->getID() << "\n... print out the robot_states list:\n";
			//		//cout << "robot->getID(): " << r->getID() << ", robot->state->id: " << r->state->id << endl;
			//		//cout << "Before adding the new robot: " << endl;
			//		//for (auto _r : *robots) {
			//		//	cout << "\t" << _r->state->toString() << endl;
			//		//}

			//		// set the incoming Robot message
			//		r->state->setActual(pMessage);

			//		// update header parameters
			//		r->init_offset_seqno = (r->state->getDesired().header().seqno() - r->state->getActual().header().seqno());
			//		r->init_offset_tm = (GetTickCount() - r->state->getActual().header().tm());

			//		// start the desired pose at the robot actual pose
			//		r->setDesiredToActual();

			//		//cout << "AFTER adding the new robot: " << endl;
			//		//for (auto _r : *robots) {
			//		//	cout << "\t" << _r->state->toString() << endl;
			//		//}
			//	}

			//	// update Actual if the robot is already initialized
			//	else {
			//		// set the incoming Robot message
			//		//cout << "	ROBOT " << r->getID() << " IS ALREADY INITIALIZED ... JUST UPDATE ACTUAL" << endl;
			//		r->state->setActual(pMessage);
			//	}

			//}

		}
		else {
			if (n == 0) {
				//if (robot->isInitialized())
				ofLogNotice() << "Receiving an EMPTY MESSAGE for robot: " << endl;
			}
			else {
				ofLogError() << "{n = " << n << "} error while receiving ROBOT message from SERVER " << endl;
			}
		}
	}
 */

	void updateServer() {
		if (isInitialized && has_new_message) {	// if we are connected to the server
			if (lock()) {
                
                SendMessage(body_message);
                has_new_message = false;

//                //cout << "sending dummy egm_sensor" << endl;
//                set_pose(pos_0, quat_0);
//                SendDesired(dummy_desired);
//                //cout << "receiving dummy egm_robot" << endl;
//                ReceiveActual();

				unlock();
			}
		}
	};

	bool isInitialized = false;

	void start() { startThread(); }
	void stop() { stopThread(); }

	void threadedFunction() {

//        // connect to the server
//        if (connectToServer())
//            isInitialized = true;
//        else {
//            ofLogError() << "Could not connect to the Server. Contact your system Administrator." << endl;
//            //exit();
//        }
//
//        // send out skeleton data
//
//        //// wait for message from server
//        //bool has_message = false;
//        //while (!has_message) {
//
//        //    // wait for server reply
//        //    char udpMessage[14000];
//        //    auto n = udpConnection.Receive(udpMessage, 14000);
//        //    if (n < 0) {
//        //        ofLogError() << "error while receiving message" << endl;
//        //    }
//        //    else {
//        //        cout << "Connection to Sever is setup on PORT " << portNumber << " and is ready for robots." << endl;
//        //        string message = udpMessage;
//        //        cout << "\tFirst Reply from Server: " << message << endl << endl;
//        //        has_message = true;
//        //    }
//
//        //}

		while (isThreadRunning()) {
            
			updateServer();
		}

	}

};
