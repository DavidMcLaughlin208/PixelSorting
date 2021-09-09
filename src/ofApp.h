#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include <vector>
#include <string>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		bool clickedOnLabel(const void* sender);

		void loadImage(std::string fileName);

		void resetGuiPosition();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofDirectory directory;
		ofImage image;

		ofxPanel gui;
		int guiWidth = 200;
		ofxFloatSlider slider;
		vector<ofxButton*> buttons;
};
