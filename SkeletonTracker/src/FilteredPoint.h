//  
//	FilteredPoint.h
//
//  Created by mad on 8/20/18.
//
//  Copyright (c) 2018, ATONATON, LLC. All rights reserved.

#pragma once

#include "ofMain.h"
#include "ofxOneEuroFilter.h"
#include "ofxGui.h"

/*
	Filtered 3D Point using 1€ Filter

	from http://cristal.univ-lille.fr/~casiez/1euro/
	and https://github.com/i-n-g-o/ofxOneEuroFilter

	Tuning the filter
	To minimize jitter and lag when tracking human motion, 
	the two parameters (fcmin and beta) can be set using a 
	simple two-step procedure. 
	
	First beta is set to 0 and fcmin (mincutoff) to a reasonable 
	middle-ground value such as 1 Hz. Then the body part is held 
	steady or moved at a very low speed while fcmin is adjusted 
	to remove jitter and preserve an acceptable lag during these 
	slow movements (decreasing fcmin reduces jitter but increases 
	lag, fcmin must be > 0). 
	
	Next, the body part is moved quickly in different directions 
	while beta is increased with a focus on minimizing lag. First 
	find the right order of magnitude to tune beta, which depends 
	on the kind of data you manipulate and their units: do not 
	hesitate to start with values like 0.001 or 0.0001. You can 
	first multiply and divide beta by factor 10 until you notice 
	an effect on latency when moving quickly. 

	Note that parameters fcmin and beta have clear conceptual 
	relationships: if high speed lag is a problem, increase beta; 
	if slow speed jitter is a problem, decrease fcmin.

*/

class FilteredPoint {
public:

	FilteredPoint() {}
	~FilteredPoint() {}

	void setup();
	void update(ofVec3f pos);
	void draw();
	void reset();

	ofParameterGroup get_gui() { return params; }
	ofVec3f get_raw() { return pos_raw; }
	ofVec3f get_filtered() { return pos_filtered; }

	bool isValid() { return isValidPt; }

private:
	
	ofxOneEuroFilter filter_x, filter_y, filter_z;
	void setup_filters();
	
	ofVec3f pos_raw, pos_prev_raw;
	ofVec3f pos_filtered;

	// true if filter pos != {0,0,0}
	bool isValidPt = false;

	// true if incoming pos != {0,0,0}
	bool is_valid_point(ofVec3f pos);
	void update_filtered_point();

	vector<ofVec3f> trail_raw;
	vector<ofVec3f> trail_filtered;

	void setup_gui();
	ofParameterGroup params;
	ofParameter<bool> show_filtered;
	ofParameter<bool> show_raw;
	ofParameter<double> frequency;
	ofParameter<double> mincutoff;
	ofParameter<double> beta;
	ofParameter<double> dcutoff;

	void listener_frequency(double &val);
	void listener_mincutoff(double &val);
	void listener_beta(double &val);
	void listener_dcutoff(double &val);
};