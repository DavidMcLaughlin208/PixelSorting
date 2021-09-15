#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	setupGui();
	setupShaders();

}

//--------------------------------------------------------------
void ofApp::update() {
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
	threshold = thresholdSlider;
	horizontal = horizontalToggle;
	reverse = reverseSort;
	if (started) {
		if (useCompute) {
			int maxIndex = this->horizontal ? image.getHeight() : image.getWidth();
			int widthOrHeight = this->horizontal ? image.getWidth() : 0;
			pixelSortCompute.begin();
			pixelSortCompute.setUniform1i("maxIndex", maxIndex);
			pixelSortCompute.setUniform1i("widthOrHeight", maxIndex);
			pixelSortCompute.setUniform1i("imageHeight", image.getHeight());
			pixelSortCompute.setUniform1i("imageWidth", image.getWidth());
			pixelSortCompute.setUniform1i("bytesPerPixel", pixels.getBytesPerPixel());
			pixelSortCompute.setUniform1i("reverse", this->reverse);
			pixelSortCompute.setUniform1i("horizontal", this->horizontal);
			pixelSortCompute.setUniform1f("threshold", this->threshold);
			pixelSortCompute.dispatchCompute((image.getHeight() * image.getWidth() + 1024 - 1) / 1024, 1, 1);
			pixelSortCompute.end();
			started = false;

			glm::vec3* updatedPixels = pixelsBuffer.map<glm::vec3>(GL_READ_WRITE);
			vector<unsigned char> charVec;
			convertVecToCharPixels(charVec, updatedPixels, pixels.getBytesPerPixel(), image.getWidth() * image.getHeight());
			unsigned char* startOfPixels = &(charVec[0]);
			pixels.setFromPixels(startOfPixels, image.getWidth(), image.getHeight(), pixels.getPixelFormat());
			pixelsBuffer.unmap();
			image.setFromPixels(pixels);
		}
		else {
			pixelSort();
		}
	}
}

void ofApp::convertVecToCharPixels(vector<unsigned char> &charVec, glm::vec3* vecPointer, int bytesPerPixel, int size) {
	charVec.resize(size * bytesPerPixel);
	for (int i = 0; i < size; i++) {
		if (i == 27648) {
			int p = 0;
		}
		glm::vec3 vecPixel = vecPointer[i];
		int scaledI = charVec.size() - bytesPerPixel - (i * bytesPerPixel);
		charVec[scaledI] = vecPixel.r * 255.0f;
		charVec[scaledI + 1] = vecPixel.g * 255.0f;
		charVec[scaledI + 2] = vecPixel.b * 255.0f;
		if (bytesPerPixel == 4) {
			charVec[scaledI + 3] = 255;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	if (!useCompute) {
		if (image.isAllocated()) {

			int wid = image.getWidth();
			int hei = image.getHeight();
			if (image.getWidth() > 1280 || image.getHeight() > 768) {
				int heightRat = image.getHeight() / 768.0f;
				int widthRat = image.getWidth() / 1280.0f;
				if (widthRat > heightRat) {
					wid = 1280;
					hei = image.getHeight() / widthRat;
				}
				else {
					wid = image.getWidth() / heightRat;
					hei = 768;
				}
			}
			image.draw(0, 0, wid, hei);
		}
	}
	else {
		/*shader.begin();
		shader.setUniform1i("bytesPerPixel", pixels.getBytesPerPixel());
		shader.setUniform1i("imageWidth", image.getWidth());
		shader.setUniform1i("imageHeight", image.getHeight());
		ofDrawRectangle(0, 0, image.getWidth(), image.getHeight());
		shader.end();*/
		image.draw(0, 0);
	}
	gui.draw();
}

void ofApp::loadImage(std::string fileName) {
	texture.unbind();
	sortedTexture.clear();
	sortedTexture.unbind();

	image.load("images/" + fileName);
	currentFileName = fileName;
	/*texture = image.getTexture();
	texture.bindAsImage(0, GL_READ_ONLY);
	sortedTexture = texture;
	sortedTexture.bindAsImage(1, GL_WRITE_ONLY);*/
	/*int wid = image.getWidth();
	int hei = image.getHeight();
	if (image.getWidth() > 1280 || image.getHeight() > 768) {
		int heightRat = image.getHeight() / 768.0f;
		int widthRat = image.getWidth() / 1280.0f;
		if (widthRat > heightRat) {
			wid = 1280;
			hei = image.getHeight() / widthRat;
		}
		else {
			wid = image.getWidth() / heightRat;
			hei = 768;
		}
	}*/
	ofSetWindowShape(image.getWidth() + guiWidth, image.getHeight());
	resetGuiPosition();
	pixels = image.getPixels();
	if (useCompute) {
		int bpp = pixels.getBytesPerPixel();
		pixelAllocater.resize(image.getWidth() * image.getHeight());
		pixels.mirror(true, false);
		for (int i = 0; i < pixelAllocater.size(); i++) {
			glm::vec3 & bufferPixel = pixelAllocater[i];
			int y = int((float)i / (float)image.getWidth());
			int x = i - y * image.getWidth();
			ofColor color = pixels.getColor(x, y);
			bufferPixel.r = color.r / 255.0f;
			bufferPixel.g = color.g / 255.0f;
			bufferPixel.b = color.b / 255.0f;
		}
		if (pixelsBuffer.isAllocated()) {
			pixelsBuffer.invalidate();
		}
		pixelsBuffer.allocate(pixelAllocater, GL_DYNAMIC_DRAW);
		pixelsBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
		//int bufferSize = image.getWidth() * image.getHeight() * sizeof(glm::uint);
		//pixelsBuffer.allocate(image.getWidth() * image.getHeight() * 3, GL_READ_WRITE);
		/*for (int i = 0; i < bufferSize; i++) {

		}*/
		//pixelsBuffer.bind(GL_PIXEL_UNPACK_BUFFER);
	}
	sortingIndex = 0;
	started = false;
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
		float value = color.getBrightness() / 255.0f;// getThresholdVariableFromColor(color, currentlySelectedThresholdVariable);
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
				float val = color.getBrightness() / 255.0f;// getThresholdVariableFromColor(color, currentlySelectedThresholdVariable);
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
		return (index * imageWidth) + (column);
	}
}

float ofApp::getThresholdVariableFromColor(ofColor color, std::string selectedVariable) {
	return color.getBrightness() / 255.0f;
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

void ofApp::resetGuiPosition() {
	gui.setPosition(ofGetWidth() - guiWidth, 10);
}

void ofApp::setupGui() {
	gui.setup();
	gui.setPosition(ofGetWidth() - guiWidth, 10);
	gui.add(sortButton.setup("Sort"));
	sortButton.addListener(this, &ofApp::start);
	gui.add(saveButton.setup("Save Image"));
	saveButton.addListener(this, &ofApp::saveCurrentImage);
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

	// May want to separate this to a function at some point
	directory.open("images");
	directory.listDir();
	for (int i = 0; i < directory.size(); i++) {
		ofxButton* button = new ofxButton();
		gui.add(button->setup(directory.getName(i)));
		buttons.push_back(button);
		button->addListener(this, &ofApp::clickedOnLabel);
	}
}

void ofApp::setupShaders() {
	pixelSortCompute.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/pixelSort.compute");
	pixelSortCompute.linkProgram();
	shader.load("shaders/shader");
}

void ofApp::saveCurrentImage() {
	if (image.isAllocated()) {
		ofFilePath filePath;
		std::string fileName = filePath.getBaseName(currentFileName);
		std::string extension = "." + filePath.getFileExt(currentFileName);
		std::string fullName = fileName + "1" + extension;
		image.save("images/" + fullName);
		currentFileName = fullName;
	}
	
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
