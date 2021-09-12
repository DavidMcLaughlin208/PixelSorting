#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	gui.setup();
	gui.setPosition(ofGetWidth() - guiWidth, 10);
	gui.add(sortButton.setup("Sort"));
	sortButton.addListener(this, &ofApp::start);
	gui.add(thresholdSlider.setup("Threshold", 0.5, 0.0, 1.0));
	gui.add(horiztonalToggleLabel.setup((std::string) "Horzontal Sort"));
	gui.add(horizontalToggle.setup("Horizontal"));
	gui.add(reverseSortLabel.setup((string)"Reverse Direction"));
	gui.add(reverseSort.setup("Reverse Sort"));

	currentlySelectedThresholdVariable = BRIGHTNESS;
	gui.add(selectedThresholdVariable.setup((string)"Sorting by: " + currentlySelectedThresholdVariable));
	gui.add(brightnessRadio.setup(BRIGHTNESS));
	gui.add(lightnessRadio.setup(LIGHTNESS));
	gui.add(hueRadio.setup(HUE));
	gui.add(saturationRadio.setup(SATURATION));
	brightnessRadio.addListener(this, &ofApp::selectParameterRadioButton);
	lightnessRadio.addListener(this, &ofApp::selectParameterRadioButton);
	hueRadio.addListener(this, &ofApp::selectParameterRadioButton);
	saturationRadio.addListener(this, &ofApp::selectParameterRadioButton);

	

	directory.open("images");
	directory.listDir();
	for (int i = 0; i < directory.size(); i++) {
		ofxButton* button = new ofxButton();
		gui.add(button->setup(directory.getName(i)));
		buttons.push_back(button);
		button->addListener(this, &ofApp::clickedOnLabel);
	}
	
	
}

//--------------------------------------------------------------
void ofApp::update() {
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
	threshold = thresholdSlider;
	horizontal = horizontalToggle;
	reverse = reverseSort;
	if (started) {
		pixelSort();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	if (image.isAllocated()) {
		image.draw(0, 0);
	}
	gui.draw();
}

void ofApp::pixelSort() {
	int widthOrHeight;
	int start = sortingIndex;
	int end;
	int columnsOrRows;
	if (horizontal) {
		widthOrHeight = image.getWidth();
		end = (start + 1) * widthOrHeight;
		columnsOrRows = image.getHeight();
	}
	else {
		widthOrHeight = 0;
		end = image.getHeight();
		columnsOrRows = image.getWidth();
	}
	float highestVal;
	int indexOfHighest;
	
	int bytesPerPixel = pixels.getBytesPerPixel();

	int startOfInterval = -1;
	int endOfInterval = -1;

	ofColor color;
	char colorArray[4];
	for (int i = start * widthOrHeight; i < end; i++) {
		int actualI = getActualIndex(i, start, bytesPerPixel, image.getWidth(), this->horizontal);
		for (int c = 0; c < bytesPerPixel; c++) {
			colorArray[c] = pixels[actualI + c];
		}
		color.set(colorArray[0], colorArray[1], colorArray[2]);
		float value = getThresholdVariableFromColor(color, currentlySelectedThresholdVariable);
		if (value >= threshold) {
			if (startOfInterval == -1) {
				startOfInterval = i;
				continue;
			}
			else {
				// If we are above threshold and we already have a valid startOfInterval
				// then we extend endOfInterval to the current index, we have special logic here 
				// to ensure that if we are at the end of a row we start the currently started interval
				endOfInterval = i;
				if (!(i == end - 1)) {
					continue;
				}
				else {
					// this is the end of a row or column so we will sort this interval
				}
			}
		}
		else {
			if (startOfInterval == -1) {
				// If we are under threshold and there is no start of interval then there is nothing to do here
				continue;
			}
			else {
				// If we are below threshold and we have a valid startOfInterval index
				// then this means that we have found our interval and we will move on
				// to sort that interval in the following nested loop
			}
		}
		
		for (int s = startOfInterval; s <= endOfInterval; s++) {
			int modS = s;
			if (reverse) {
				modS = endOfInterval - s + startOfInterval;
			}
			int actualS = getActualIndex(modS, start, bytesPerPixel, image.getWidth(), this->horizontal);
			indexOfHighest = actualS;
			highestVal = -1;
			for (int j = s; j <= endOfInterval; j++) {
				int modJ = j;
				if (reverse) {
					modJ = endOfInterval - j + startOfInterval;
				}
				int actualJ = getActualIndex(modJ, start, bytesPerPixel, image.getWidth(), this->horizontal);
				for (int c = 0; c < bytesPerPixel; c++) {
					colorArray[c] = pixels[actualJ + c];
				}
				color.set(colorArray[0], colorArray[1], colorArray[2]);
				float val = getThresholdVariableFromColor(color, currentlySelectedThresholdVariable);
				if (val > highestVal) {
					highestVal = val;
					indexOfHighest = actualJ;
				}
			}
			swapPixels(pixels, actualS, indexOfHighest, bytesPerPixel);
		}
		startOfInterval = -1;
		endOfInterval = -1;
	}

	sortingIndex += 1;
	if (sortingIndex >= columnsOrRows - 1) {
		sortingIndex = 0;
		started = false;
	}
	image.setFromPixels(pixels);
}

void ofApp::swapPixels(ofPixels &pixels, int index1, int index2, int bytesPerPixel) {
	for (int c = 0; c < bytesPerPixel; c++) {
		pixelSwapBuffer[c] = pixels[index1 + c];
	}
	for (int c = 0; c < bytesPerPixel; c++) {
		pixels[index1 + c] = pixels[index2 + c];
	}
	for (int c = 0; c < bytesPerPixel; c++) {
		pixels[index2 + c] = pixelSwapBuffer[c];
	}
}

int ofApp::getActualIndex(int index, int column, int bytesPerPixel, int imageWidth, bool isHorizontal) {
	if (isHorizontal) {
		return index * bytesPerPixel;
	}
	else {
		return (index * imageWidth * bytesPerPixel) + (column * bytesPerPixel);
	}
}

float ofApp::getThresholdVariableFromColor(ofColor color, std::string selectedVariable) {
	if (selectedVariable == BRIGHTNESS) {
		return color.getBrightness() / 255.0f;
	}
	else if (selectedVariable == LIGHTNESS) {
		return color.getLightness() / 255.0f;
	}
	else if (selectedVariable == HUE) {
		return color.getHue() / 255.0f;
	}
	else if (selectedVariable == SATURATION) {
		return color.getSaturation() / 255.0f;
	}
	else {
		ofLogError((string)"Invalid selected threshold variable: " + selectedVariable);
		return 0.0f;
	}
}

void ofApp::start() {
	started = !started;
}

void ofApp::selectParameterRadioButton(const void* sender) {
	currentlySelectedThresholdVariable = ((ofParameter<bool>*)sender)->getName();
	selectedThresholdVariable = (string)"Sorting by: " + currentlySelectedThresholdVariable;
}

bool ofApp::clickedOnLabel(const void* sender) {
	ofParameter<bool>* button = (ofParameter<bool>*)sender;
	loadImage(button->getName());
	return true;
}

void ofApp::loadImage(std::string fileName) {
	image.load("images/" + fileName);
	ofSetWindowShape(image.getWidth() + guiWidth, image.getHeight());
	resetGuiPosition();
	pixels = image.getPixels();
	sortingIndex = 0;
	started = false;
}

void ofApp::resetGuiPosition() {
	gui.setPosition(ofGetWidth() - guiWidth, 10);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
