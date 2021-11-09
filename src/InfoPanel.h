#pragma once


#include "ofxDatGui.h"
#include <vector>

class InfoPanel {
	private:
		vector<ofxDatGuiComponent*> items;
		ofxDatGuiLabel* activeStatusLabel;
		ofxDatGuiLabel* usingMaskLabel;
		ofxDatGuiSlider* progressBar;
		ofxDatGuiLabel* estTimeToCompletion;
		ofxDatGuiLabel* frameCount;
		ofxDatGuiLabel* lastSortTimeTakenLabel;
		vector<int> lastSortTime;
		int sortTimeIndex = 0;

	public:
		void setup();
		void setActiveStatus(std::string);
		void setItemPositions(int xAnchor, int yAnchor, int totalWidth);
		void drawItems();
		void setUsingMask(bool useMask);
		void setProgress(float percentage);
		void sortTimeTaken(int milliseconds);
		void setFrameCounter(int currentFrame, int totalFrames);
		int getAverageSortTime();

};