#include "ofApp.h"

std::string ofApp::BRIGHTNESS = "Brightness";
std::string ofApp::LIGHTNESS = "Lightness";
std::string ofApp::HUE = "Hue";
std::string ofApp::SATURATION = "Saturation";

void pixelSortRow(int startIndex, bool horizontal, bool reverse, int imageWidth, int imageHeight, ofPixels& pixelsRef, std::string thresholdName, float threshold) {
	int widthOrHeight;
	int start = startIndex;
	int end;
	int columnsOrRows;
	if (horizontal) {
		widthOrHeight = imageWidth;
		end = (start + 1) * widthOrHeight;
		columnsOrRows = imageHeight;
	}
	else {
		widthOrHeight = 0;
		end = imageHeight;
		columnsOrRows = imageWidth;
	}
	float highestVal;
	int indexOfHighest;

	int bytesPerPixel = pixelsRef.getBytesPerPixel();

	int startOfInterval = -1;
	int endOfInterval = -1;

	ofColor color;
	char pixelSwapBuffer[4];
	for (int i = start * widthOrHeight; i < end; i++) {
		int actualI;
		if (horizontal) {
			actualI = i * bytesPerPixel;
		}
		else {
			actualI = i * imageWidth * bytesPerPixel + (start * bytesPerPixel);
		}
		color.set(pixelsRef[actualI + 0], pixelsRef[actualI + 1], pixelsRef[actualI + 2]);
		float value;
		if (thresholdName == ofApp::BRIGHTNESS) {
			value = color.getBrightness() / 255.0f;
		}
		else if (thresholdName == ofApp::LIGHTNESS) {
			value = color.getLightness() / 255.0f;
		}
		else if (thresholdName == ofApp::HUE) {
			value = color.getHue() / 255.0f;
		}
		else if (thresholdName == ofApp::SATURATION) {
			value = color.getSaturation() / 255.0f;
		}
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
			int actualS;
			if (horizontal) {
				actualS = modS * bytesPerPixel;
			}
			else {
				actualS = modS * imageWidth * bytesPerPixel + (start * bytesPerPixel);
			}
			indexOfHighest = actualS;
			highestVal = -1;
			for (int j = s; j <= endOfInterval; j++) {
				int modJ = j;
				if (reverse) {
					modJ = endOfInterval - j + startOfInterval;
				}
				int actualJ;
				if (horizontal) {
					actualJ = modJ * bytesPerPixel;
				}
				else {
					actualJ = modJ * imageWidth * bytesPerPixel + (start * bytesPerPixel);
				}

				color.set(pixelsRef[actualJ + 0], pixelsRef[actualJ + 1], pixelsRef[actualJ + 2]);
				float val;
				if (thresholdName == ofApp::BRIGHTNESS) {
					val = color.getBrightness() / 255.0f;
				}
				else if (thresholdName == ofApp::LIGHTNESS) {
					val = color.getLightness() / 255.0f;
				}
				else if (thresholdName == ofApp::HUE) {
					val = color.getHue() / 255.0f;
				}
				else if (thresholdName == ofApp::SATURATION) {
					val = color.getSaturation() / 255.0f;
				}
				if (val > highestVal) {
					highestVal = val;
					indexOfHighest = actualJ;
				}
			}
			for (int c = 0; c < bytesPerPixel; c++) {
				pixelSwapBuffer[c] = pixelsRef[actualS + c];
			}
			for (int c = 0; c < bytesPerPixel; c++) {
				pixelsRef[actualS + c] = pixelsRef[indexOfHighest + c];
			}
			for (int c = 0; c < bytesPerPixel; c++) {
				pixelsRef[indexOfHighest + c] = pixelSwapBuffer[c];
			}
		}
		startOfInterval = -1;
		endOfInterval = -1;
	}
}

//--------------------------------------------------------------
void ofApp::setup() {
	setupGui();
	setupShaders();

	videoExtensions.insert(".mp4");
	videoExtensions.insert(".mov");

	imageExtensions.insert(".png");
	imageExtensions.insert(".jpg");

}

//--------------------------------------------------------------
void ofApp::update() {
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
	threshold = thresholdSlider;
	horizontal = horizontalToggle;
	reverse = reverseSort;
	threadCount = threadCountSlider;
	if (started) {
		vector<std::thread> threadList;
		for (int i = 0; i < threadCount; i++) {
			threadList.push_back(std::thread(pixelSortRow, sortingIndex, this->horizontal, this->reverse, image.getWidth(), image.getHeight(), std::ref(pixels), currentlySelectedThresholdVariable, threshold));
			sortingIndex += 1;

			int columnsOrRows = this->horizontal ? image.getHeight() : image.getWidth();
			if (sortingIndex >= columnsOrRows - 1) {
				sortingIndex = 0;
				sortComplete = true;
				timeEnd = std::chrono::high_resolution_clock::now();

				long timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count();
				std::cout << "Thread Count of " << threadCount << " took " << timeTaken << " milliseconds." << std::endl;
				break;
			}
		}
		for (int i = 0; i < threadList.size(); i++) {
			threadList[i].join();
		}
		image.setFromPixels(pixels);



		if (sortComplete) {
			sortComplete = false;
			if (currentMode == Mode::Image) {
				started = false;
			}
			else if (currentMode == Mode::Video) {
				saveFrameToVideo();
				if (videoPlayer.getCurrentFrame() >= videoPlayer.getTotalNumFrames() - 1) {
					videoPlayer.close();
					videoWriter.release();
					started = false;
					std::cout << "Completed sorting video" << std::endl;
				}
				else {
					videoPlayer.nextFrame();
					videoPlayer.update();
					pixels = videoPlayer.getPixels();
					image.clear();
					image.setFromPixels(pixels);
					std::cout << "Starting frame " << videoPlayer.getCurrentFrame() << " out of " << videoPlayer.getTotalNumFrames() << std::endl;
					timeStart = std::chrono::high_resolution_clock::now();
				}
			}
		}

		
	}
}

void ofApp::saveFrameToVideo() {
	int size = image.getWidth() * image.getHeight();
	int bpp = pixels.getBytesPerPixel();
	cv::Mat mat;
	mat = cv::Mat_<cv::Vec3b>(image.getHeight(), image.getWidth());
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((float)i / (float)image.getWidth());
		int x = i - y * image.getWidth();
		mat.at<cv::Vec3b>(y, x)[0] = pixels[actualI + 2];
		mat.at<cv::Vec3b>(y, x)[1] = pixels[actualI + 1];
		mat.at<cv::Vec3b>(y, x)[2] = pixels[actualI + 0];
	}
	videoWriter.write(mat);
}

void ofApp::convertVecToCharPixels(vector<unsigned char> &charVec, glm::vec3* vecPointer, int bytesPerPixel, int size) {
	charVec.resize(size * bytesPerPixel);
	for (int i = 0; i < size; i++) {
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
			/*if (image.getWidth() > 1280 || image.getHeight() > 768) {
				int heightRat = image.getHeight() / 768.0f;
				int widthRat = image.getWidth() / 1280.0f;
				if (widthRat > heightRat) {
					wid = 1280;
					hei = image.getHeight() / heightRat;
				}
				else {
					wid = image.getWidth() / widthRat;
					hei = 768;
				}
			}*/
			image.draw(0, 0, wid, hei);
		}
	}
	else {
		if (image.isAllocated()) {
			image.draw(0, 0);
		}
	}
	gui.draw();
}

void ofApp::loadImage(std::string fileName) {
	videoPlayer.close();
	image.clear();
	ofFilePath filePath;
	std::string extension = "." + filePath.getFileExt(fileName);

	if (imageExtensions.find(extension) != imageExtensions.end()) {
		image.load("images/" + fileName);
		pixels = image.getPixels();
		currentMode = Mode::Image;
	}
	else if (videoExtensions.find(extension) != videoExtensions.end()) {
		if (videoPlayer.isLoaded()) {
			videoPlayer.close();
		}
		if (videoWriter.isOpened()) {
			videoWriter.release();
		}
		videoPlayer.load("images/" + fileName);
		videoPlayer.firstFrame();
		videoPlayer.play();
		videoPlayer.setPaused(true);
		pixels = videoPlayer.getPixels();
		image.setFromPixels(pixels);

		float fps = videoPlayer.getTotalNumFrames() / videoPlayer.getDuration();
		videoWriter = cv::VideoWriter("data/images/effect.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(image.getWidth(), image.getHeight()), true);
		currentMode = Mode::Video;
	}
	else {
		//Display some error message
		currentMode = Mode::None;
		return;
	}
	currentFileName = fileName;
	int wid = image.getWidth();
	int hei = image.getHeight();
	/*if (image.getWidth() > 1280 || image.getHeight() > 768) {
		int heightRat = image.getHeight() / 768.0f;
		int widthRat = image.getWidth() / 1280.0f;
		if (widthRat > heightRat) {
			wid = 1280;
			hei = image.getHeight() / heightRat;
		}
		else {
			wid = image.getWidth() / widthRat;
			hei = 768;
		}
	}*/
	ofSetWindowShape(image.getWidth() + guiWidth, image.getHeight());
	resetGuiPosition();
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
	//char colorArray[4];
	for (int i = start * widthOrHeight; i < end; i++) {
		int actualI;// = getActualIndex(i, start, bytesPerPixel, image.getWidth(), this->horizontal);
		if (this->horizontal) {
			actualI = i * bytesPerPixel;
		}
		else {
			actualI = i * image.getWidth() * bytesPerPixel + (start * bytesPerPixel);
		}
		color.set(pixels[actualI + 0], pixels[actualI + 1], pixels[actualI + 2]);
		float value;
		//Inlining this beccause separating as a function is really slow
		if (currentlySelectedThresholdVariable == BRIGHTNESS) {
			value = color.getBrightness() / 255.0f;
		}
		else if (currentlySelectedThresholdVariable == LIGHTNESS) {
			value = color.getLightness() / 255.0f;
		}
		else if (currentlySelectedThresholdVariable == HUE) {
			value = color.getHue() / 255.0f;
		}
		else if (currentlySelectedThresholdVariable == SATURATION) {
			value = color.getSaturation() / 255.0f;
		}
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
			//Inlining this function because c++
			int actualS;// = getActualIndex(modS, start, bytesPerPixel, image.getWidth(), this->horizontal);
			if (this->horizontal) {
				actualS = modS * bytesPerPixel;
			}
			else {
				actualS = modS * image.getWidth() * bytesPerPixel + (start * bytesPerPixel);
			}
			indexOfHighest = actualS;
			highestVal = -1;
			for (int j = s; j <= endOfInterval; j++) {
				int modJ = j;
				if (reverse) {
					modJ = endOfInterval - j + startOfInterval;
				}
				//Inlining this function because c++
				int actualJ;// = getActualIndex(modJ, start, bytesPerPixel, image.getWidth(), this->horizontal);
				if (this->horizontal) {
					actualJ = modJ * bytesPerPixel;
				}
				else {
					actualJ = modJ * image.getWidth() * bytesPerPixel + (start * bytesPerPixel);
				}
				/*for (int c = 0; c < bytesPerPixel; c++) {
					colorArray[c] = pixels[actualJ + c];
				}*/
				color.set(pixels[actualJ + 0], pixels[actualJ + 1], pixels[actualJ + 2]);
				float val;
				if (currentlySelectedThresholdVariable == BRIGHTNESS) {
					val = color.getBrightness() / 255.0f;
				}
				else if (currentlySelectedThresholdVariable == LIGHTNESS) {
					val = color.getLightness() / 255.0f;
				}
				else if (currentlySelectedThresholdVariable == HUE) {
					val = color.getHue() / 255.0f;
				}
				else if (currentlySelectedThresholdVariable == SATURATION) {
					val = color.getSaturation() / 255.0f;
				}
				if (val > highestVal) {
					highestVal = val;
					indexOfHighest = actualJ;
				}
			}
			for (int c = 0; c < bytesPerPixel; c++) {
				pixelSwapBuffer[c] = pixels[actualS + c];
			}
			for (int c = 0; c < bytesPerPixel; c++) {
				pixels[actualS + c] = pixels[indexOfHighest + c];
			}
			for (int c = 0; c < bytesPerPixel; c++) {
				pixels[indexOfHighest + c] = pixelSwapBuffer[c];
			}
			//swapPixels(pixels, actualS, indexOfHighest, bytesPerPixel);
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
	if (started) {
		timeStart = std::chrono::high_resolution_clock::now();
	}
	else {
		videoWriter.release();
	}
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
	gui.add(threadCountSlider.setup("Thread Count", 17, 0, 30));

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



