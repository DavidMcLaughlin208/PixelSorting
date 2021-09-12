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
		void pixelSort();
		void swapPixels(ofPixels& pixels, int index1, int index2, int bytesPerPixel);
		int getActualIndex(int index, int column, int bytesPerPixel, int imageWidth, bool isHorizontal);
		float getThresholdVariableFromColor(ofColor color, std::string selectedVariable);
		void start();
		void selectParameterRadioButton(const void* sender);
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

		std::string BRIGHTNESS = "Brightness";
		std::string LIGHTNESS = "Lightness";
		std::string HUE = "Hue";
		std::string SATURATION = "Saturation";
		
		ofDirectory directory;
		ofImage image;
		ofImage sortedImage;
		ofPixels pixels;

		ofxPanel gui;
		int guiWidth = 200;
		ofxFloatSlider thresholdSlider;
		vector<ofxButton*> buttons;
		ofxButton sortButton;
		ofxToggle horizontalToggle;
		ofxToggle reverseSort;
		ofxLabel reverseSortLabel;
		ofxLabel horiztonalToggleLabel;
		char pixelSwapBuffer[4];

		int sortingIndex = 0;
		bool started = false;

		float threshold = 0.1f;
		bool horizontal = false;
		bool reverse = false;

		// Threshold parameter radio buttons
		ofxButton brightnessRadio;
		ofxButton hueRadio;
		ofxButton lightnessRadio;
		ofxButton saturationRadio;
		vector<ofxButton> thresholdValueRadioButtons;
		std::string currentlySelectedThresholdVariable;
		ofxLabel selectedThresholdVariable;


};
