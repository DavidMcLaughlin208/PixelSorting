#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	gui.setup();
	gui.setPosition(ofGetWidth() - guiWidth, 10);
	gui.add(sortButton.setup("Sort"));
	sortButton.addListener(this, &ofApp::start);

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
	if (started) {
		pixelSort();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	if (image.isAllocated()) {
		image.draw(0, 0);
	}
	if (sortedImage.isAllocated()) {
		sortedImage.draw(image.getWidth(), 0);
	}
	gui.draw();
}

void ofApp::pixelSort() {
	//int i = sortingIndex;
	int width = image.getWidth();
	float highest;
	int indexOfHighest;
	int startingIndex = sortingIndex;
	int bytesPerPixel = pixels.getBytesPerPixel();
	ofColor color;
	int columns = image.getHeight();
	char colorArray[4];
	for (int i = startingIndex * width; i <= (startingIndex + 1) * width; i++) {
		int actualI = i * bytesPerPixel;
		indexOfHighest = actualI;
		highest = -1;
		for (int j = i; j < (startingIndex + 1) * width; j ++) {
			int actualJ = j * bytesPerPixel;
			for (int c = 0; c < bytesPerPixel; c++) {
				colorArray[c] = pixels[actualJ + c];
			}
			char r = pixels[actualJ];
			char g = pixels[actualJ + 1];
			char b = pixels[actualJ + 2];
			color.set(colorArray[0], colorArray[1], colorArray[2]);
			float val = color.getBrightness();
			if (val > highest) {
				highest = val;
				indexOfHighest = actualJ;
			}
		}
		swapPixels(pixels, actualI, indexOfHighest, bytesPerPixel);
		/*char oldR = pixels[i];
		char oldG = pixels[i + 1];
		char oldB = pixels[i + 2];
		char oldA = pixels[i + 3];

		pixels[i] = pixels[indexOfHighest];
		pixels[i + 1] = pixels[indexOfHighest + 1];
		pixels[i + 2] = pixels[indexOfHighest + 2];
		pixels[i + 3] = pixels[indexOfHighest + 3];

		pixels[indexOfHighest] = oldR;
		pixels[indexOfHighest + 1] = oldG;
		pixels[indexOfHighest + 2] = oldB;
		pixels[indexOfHighest + 3] = oldA;*/
	}

	sortingIndex += 1;
	//ofLogNotice("Finished sorting row " + sortingIndex);
	if (sortingIndex >= columns) {
		sortingIndex = 0;
		started = false;
		ofLogNotice("Finished sorting");
	}
	//}
	sortedImage.setFromPixels(pixels);
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

void ofApp::start() {
	started = !started;
}

bool ofApp::clickedOnLabel(const void* sender) {
	ofParameter<bool>* button = (ofParameter<bool>*)sender;
	loadImage(button->getName());
	return true;
}

void ofApp::loadImage(std::string fileName) {
	image.load("images/" + fileName);
	ofSetWindowShape(image.getWidth() * 2 + guiWidth, image.getHeight());
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
	ofLogNotice("Released");
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
