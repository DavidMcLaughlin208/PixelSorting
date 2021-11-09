#include "InfoPanel.h"

void InfoPanel::setup() {
	ofxDatGuiTheme* theme = new ofxDatGuiTheme();
	theme->color.slider.fill = ofColor(255, 255, 255);
	theme->init();
	activeStatusLabel = new ofxDatGuiLabel("Status: Idle");
	usingMaskLabel = new ofxDatGuiLabel("Using Mask: false");
	progressBar = new ofxDatGuiSlider("Progress", 0, 100, 0);
	// I can't figure out why the progress bar fill is not being drawn
	progressBar->setScale(1);
	progressBar->setTheme(theme);
	lastSortTimeTakenLabel = new ofxDatGuiLabel("Last Sort Duration: ");
	frameCount = new ofxDatGuiLabel("Frame: 0 / 0");
	estTimeToCompletion = new ofxDatGuiLabel("Est Time Remaining: ");
	items.push_back(activeStatusLabel);
	items.push_back(usingMaskLabel);
	items.push_back(progressBar);
	items.push_back(lastSortTimeTakenLabel);
	items.push_back(frameCount);
	items.push_back(estTimeToCompletion);
	
	for (int i = 0; i < items.size(); i++) {
		items[i]->setLabelUpperCase(false);
	}
}

void InfoPanel::setItemPositions(int xAnchor, int yAnchor, int totalWidth) {
	int step = totalWidth / items.size();
	for (int i = 0; i < items.size(); i++) {
		items[i]->setPosition(i * step, 0);
		items[i]->setWidth(step, 75);
	}
}

void InfoPanel::setActiveStatus(std::string status) {
	activeStatusLabel->setLabel("Status: " + status);
}

void InfoPanel::setUsingMask(bool useMask) {
	usingMaskLabel->setLabel("Using Mask: " + useMask ? "True" : "False");
}

void InfoPanel::setProgress(float percentage) {
	
	progressBar->setValue(percentage * 100);
}

void InfoPanel::drawItems() {
	for (int i = 0; i < items.size(); i++) {
		items[i]->draw();
	}
}

void InfoPanel::sortTimeTaken(int milliseconds) {
	
	if (lastSortTime.size() >= 10) {
		lastSortTime[sortTimeIndex] = milliseconds;
		sortTimeIndex++;
		sortTimeIndex = sortTimeIndex > 9 ? 0 : sortTimeIndex;
	}
	else {
		lastSortTime.push_back(milliseconds);
	}
	lastSortTimeTakenLabel->setLabel("Last Sort Duration: " + ofToString(milliseconds) + "ms");
}

void InfoPanel::setFrameCounter(int currentFrame, int totalFrames) {
	frameCount->setLabel("Frame " + ofToString(currentFrame) + "/" + ofToString(totalFrames));
	int secondsRemaining = getAverageSortTime() * (totalFrames - currentFrame) / 1000;
	int minutesRemaining = secondsRemaining / 60;
	estTimeToCompletion->setLabel("Est Time Remaining: " + ofToString(minutesRemaining) + "m " + ofToString(secondsRemaining % 60) + "s");
}

int InfoPanel::getAverageSortTime() {
	int sum = 0;
	if (lastSortTime.size() == 0) {
		return sum;
	}
	for (int i = 0; i < lastSortTime.size(); i++) {
		sum += lastSortTime[i];
	}
	return (float) sum / (float) lastSortTime.size();
}