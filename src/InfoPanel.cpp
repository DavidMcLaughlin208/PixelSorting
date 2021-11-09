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
	items.push_back(activeStatusLabel);
	items.push_back(usingMaskLabel);
	items.push_back(progressBar);
	items.push_back(lastSortTimeTakenLabel);
	
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
	usingMaskLabel->setLabel("Using Mask: " + ofToString(useMask));
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
	lastSortTimeTakenLabel->setLabel("Last Sort Duration: " + ofToString(milliseconds) + "ms");
}