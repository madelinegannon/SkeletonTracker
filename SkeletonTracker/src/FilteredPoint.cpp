#include "FilteredPoint.h"

//--------------------------------------------------------------
void FilteredPoint::setup()
{
	setup_gui();

	setup_filters();
}

//--------------------------------------------------------------
void FilteredPoint::update(ofVec3f pos)
{
	// update the previous and current raw pos
	pos_prev_raw.set(pos_raw);
	pos_raw.set(pos);


	if (is_valid_point(pos_raw)) {
		update_filtered_point();
		trail_filtered.push_back(pos_filtered);
		trail_raw.push_back(pos_raw);
	}


	// update trails
	if (trail_filtered.size() > 75) {
		trail_filtered.erase(trail_filtered.begin());
		trail_raw.erase(trail_raw.begin());
	}

}

//--------------------------------------------------------------
void FilteredPoint::draw()
{
	ofPushStyle();
	ofEnableDepthTest();
	if (show_filtered) {
		ofSetLineWidth(3);
		ofSetColor(ofColor::magenta, 120);
		glBegin(GL_LINE_STRIP);
		for (auto & v : trail_filtered)
			glVertex3fv(v.getPtr());
		glEnd();

		ofNoFill();
		ofSetColor(ofColor::magenta, 60);
		ofPushMatrix();
		ofTranslate(pos_filtered);
		ofBox(.025);

		ofSetColor(250);
		ofDrawBitmapStringHighlight("{ " + ofToString(pos_filtered) + " }", 20, 20);
		ofPopMatrix();
	}

	if (show_raw) {
		ofSetLineWidth(3);
		ofSetColor(ofColor::aquamarine, 40);
		glBegin(GL_LINE_STRIP);
		for (auto & v : trail_raw)
			glVertex3fv(v.getPtr());
		glEnd();

		ofSetLineWidth(1);
		ofFill();
		ofSetColor(ofColor::aqua, 60);
		ofPushMatrix();
		ofTranslate(pos_raw);
		ofBox(.07);
		ofPopMatrix();
	}
	ofDisableDepthTest();
	ofPopStyle();
}

//--------------------------------------------------------------
void FilteredPoint::reset()
{

}

//--------------------------------------------------------------
void FilteredPoint::setup_filters()
{
	filter_x.setup(frequency, mincutoff, beta, dcutoff);
	filter_y.setup(frequency, mincutoff, beta, dcutoff);
	filter_z.setup(frequency, mincutoff, beta, .1);		// z value is noisier than x or y
}

//--------------------------------------------------------------
bool FilteredPoint::is_valid_point(ofVec3f pos)
{
	return (pos.x != 0 && pos.y != 0 && pos.z != 0);
}

//--------------------------------------------------------------
void FilteredPoint::update_filtered_point()
{
	pos_filtered.x = filter_x.filter(pos_raw.x, ofGetElapsedTimef());
	pos_filtered.y = filter_y.filter(pos_raw.y, ofGetElapsedTimef());
	pos_filtered.z = filter_z.filter(pos_raw.z, ofGetElapsedTimef());

	isValidPt = (pos_filtered.x != 0 && pos_filtered.y != 0 && pos_filtered.z != 0);
}

void FilteredPoint::setup_gui()
{
	/*
		frequency = 60;
		mincutoff = .01;
		beta = .01;
		dcutoff = .95;
	*/

	params.setName("Filtered_Point");
	params.add(show_filtered.set("Show_Filtered", true));
	params.add(show_raw.set("Show_Raw", true));
	params.add(frequency.set("Frequency", 60, 0, 120));
	params.add(mincutoff.set("Min_Cutoff", .75, 0, 1));
	params.add(beta.set("Beta", .01, 0, .2));
	params.add(dcutoff.set("Derivate_Cuttoff", .6, 0, 1));

	// add listeners
	frequency.addListener(this, &FilteredPoint::listener_frequency);
	mincutoff.addListener(this, &FilteredPoint::listener_mincutoff);
	beta.addListener(this, &FilteredPoint::listener_beta);
	dcutoff.addListener(this, &FilteredPoint::listener_dcutoff);
}

//--------------------------------------------------------------
void FilteredPoint::listener_frequency(double & val)
{
	filter_x.setFrequency(frequency);
	filter_y.setFrequency(frequency);
	filter_z.setFrequency(frequency);
}

void FilteredPoint::listener_mincutoff(double & val)
{
	filter_x.setMinCutoff(mincutoff);
	filter_y.setMinCutoff(mincutoff);
	filter_z.setMinCutoff(mincutoff);
}

void FilteredPoint::listener_beta(double & val)
{
	filter_x.setBeta(beta);
	filter_y.setBeta(beta);
	filter_z.setBeta(beta);
}

void FilteredPoint::listener_dcutoff(double & val)
{
	filter_x.setDerivateCutoff(dcutoff);
	filter_y.setDerivateCutoff(dcutoff);
	filter_z.setDerivateCutoff(dcutoff);
}
//--------------------------------------------------------------