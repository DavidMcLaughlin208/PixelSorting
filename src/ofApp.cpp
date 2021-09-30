#include "ofApp.h"

std::string ofApp::BRIGHTNESS = "Brightness";
std::string ofApp::LIGHTNESS = "Lightness";
std::string ofApp::HUE = "Hue";
std::string ofApp::SATURATION = "Saturation";

struct BrightnessComparator {
	bool operator() (ofColor i, ofColor j) { return (i.getBrightness() < j.getBrightness()); }
} brightnessComparator;

struct HueComparator {
	bool operator() (ofColor i, ofColor j) { return (i.getHue() < j.getHue()); }
} hueComparator;

struct SaturationComparator {
	bool operator() (ofColor i, ofColor j) { return (i.getSaturation() < j.getSaturation()); }
} saturationComparator;

void pixelSortRow(int startIndex, bool horizontal, bool reverse, int imageWidth, int imageHeight, ofPixels& pixelsRef, ofApp::SortParameter sortParameter, float threshold, float upperThreshold) {
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
		int actualI = horizontal ? actualI = i * bytesPerPixel : i * imageWidth * bytesPerPixel + (start * bytesPerPixel);
		color.set(pixelsRef[actualI + 0], pixelsRef[actualI + 1], pixelsRef[actualI + 2], pixelsRef[actualI + 3]);
		float value = 0.0;

		switch (sortParameter) {
			case ofApp::SortParameter::Brightness:
				value = color.getBrightness() / 255.0f;
				break;

			case ofApp::SortParameter::Hue:
				value = color.getHue() / 255.0f;
				break;

			case ofApp::SortParameter::Saturation:
				value = color.getSaturation() / 255.0f;
				break;

		}

		if (color.a != 0 && (value >= threshold && value <= upperThreshold)) {
			if (startOfInterval == -1) {
				startOfInterval = i;
				continue;
			}
			else {
				if (!(i == end - 1)) {
					// This is not the end of a row/column and we are within threshold range so we will
					// continue in the loop to find the interval
					continue;
				}
				else {
					// This is the end of a row or column so we will sort this interval
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
		endOfInterval = i - 1;

		vector<ofColor> intervalColors;
		intervalColors.resize(endOfInterval - startOfInterval + 1);
		for (int s = startOfInterval; s <= endOfInterval; s++) {
			int modS = reverse ? endOfInterval - s + startOfInterval : s;
			int actualS = horizontal ? modS * bytesPerPixel : modS * imageWidth * bytesPerPixel + (start * bytesPerPixel);
			intervalColors[s - startOfInterval].r = pixelsRef[actualS + 0];
			intervalColors[s - startOfInterval].g = pixelsRef[actualS + 1];
			intervalColors[s - startOfInterval].b = pixelsRef[actualS + 2];
		}
		switch (sortParameter) {
		case ofApp::SortParameter::Brightness:
			std::sort(intervalColors.begin(), intervalColors.end(), brightnessComparator);
			break;

		case ofApp::SortParameter::Hue:
			std::sort(intervalColors.begin(), intervalColors.end(), hueComparator);
			break;

		case ofApp::SortParameter::Saturation:
			std::sort(intervalColors.begin(), intervalColors.end(), saturationComparator);
			break;

		}

		for (int s = startOfInterval; s <= endOfInterval; s++) {
			int modS = reverse ? endOfInterval - s + startOfInterval : s;
			int actualS = horizontal ? modS * bytesPerPixel : modS * imageWidth * bytesPerPixel + (start * bytesPerPixel);
			pixelsRef[actualS + 0] = intervalColors[s - startOfInterval].r;
			pixelsRef[actualS + 1] = intervalColors[s - startOfInterval].g;
			pixelsRef[actualS + 2] = intervalColors[s - startOfInterval].b;
		}
		startOfInterval = -1;
		endOfInterval = -1;
	}
}

//--------------------------------------------------------------
void ofApp::setup() {
	setupGui();

	videoExtensions.insert(".mp4");
	videoExtensions.insert(".mov");

	imageExtensions.insert(".png");
	imageExtensions.insert(".jpg");

	sortParameterTable.insert(std::pair<std::string, SortParameter>(BRIGHTNESS, SortParameter::Brightness));
	sortParameterTable.insert(std::pair<std::string, SortParameter>(HUE, SortParameter::Hue));
	sortParameterTable.insert(std::pair<std::string, SortParameter>(SATURATION, SortParameter::Saturation));

}

//--------------------------------------------------------------
void ofApp::update() {
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
	threshold = thresholdSlider;
	upperThreshold = upperThresholdSlider;
	angle = angleSlider;
	horizontal = horizontalToggle;
	reverse = reverseSort;
	threadCount = threadCountSlider;
	if (started) {
		vector<std::thread> threadList;
		for (int i = 0; i < threadCount; i++) {
			threadList.push_back(std::thread(pixelSortRow, sortingIndex, this->horizontal, this->reverse, image.getWidth(), image.getHeight(), std::ref(pixels), currentlySelectedThresholdVariable, threshold, upperThreshold));
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
				//rotateImage(-angle);
				pixels = image.getPixels();
				image.setFromPixels(pixels);
				//image.setFromPixels(pixels);
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

// Code adapted to work with OF pulled from here: https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c/33564950#33564950
void ofApp::rotateImage(int angle) {
	if (angle == 0) {
		return;
	}
	int size = image.getWidth() * image.getHeight();
	int bpp = pixels.getBytesPerPixel();
	cv::Mat src;
	src = cv::Mat_<cv::Vec4b>(image.getHeight(), image.getWidth());
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((float)i / (float)image.getWidth());
		int x = i - y * image.getWidth();
		src.at<cv::Vec4b>(y, x)[0] = pixels[actualI + 2];
		src.at<cv::Vec4b>(y, x)[1] = pixels[actualI + 1];
		src.at<cv::Vec4b>(y, x)[2] = pixels[actualI + 0];
		src.at<cv::Vec4b>(y, x)[3] = 255;
	}
	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle, center not relevant
	cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
	// adjust transformation matrix
	rot.at<double>(0, 2) += bbox.width / 2.0 - src.cols / 2.0;
	rot.at<double>(1, 2) += bbox.height / 2.0 - src.rows / 2.0;

	cv::Mat dst;
	cv::warpAffine(src, dst, rot, bbox.size());


	image.resize(dst.cols, dst.rows);
	size = image.getWidth() * image.getHeight();
	pixels.allocate(dst.cols, dst.rows, OF_IMAGE_COLOR_ALPHA);
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((float)i / (float)image.getWidth());
		int x = i - y * image.getWidth();
		pixels[actualI + 2] = dst.at<cv::Vec4b>(y, x)[0];
		pixels[actualI + 1] = dst.at<cv::Vec4b>(y, x)[1];
		pixels[actualI + 0] = dst.at<cv::Vec4b>(y, x)[2];
		pixels[actualI + 3] = dst.at<cv::Vec4b>(y, x)[3];
	}
	image.setFromPixels(pixels);
	pixels = image.getPixels();
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
		ofTranslate(image.getWidth() / 2, image.getHeight() / 2);
		ofRotate(angle);
		ofTranslate(-image.getWidth() / 2.5, -image.getHeight() / 2.5);
		image.draw(0, 0);// , wid, hei);
		ofTranslate(image.getWidth() / 2.5, image.getHeight() / 2.5);
		ofRotate(-angle);
		ofTranslate(-image.getWidth() / 2, -image.getHeight() / 2);
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
		if (image.getPixels().getBytesPerPixel() < 4) {
			pixels.allocate(image.getWidth(), image.getHeight(), OF_IMAGE_COLOR_ALPHA);
			ofPixels tempPixels = image.getPixels();
			int size = image.getWidth() * image.getHeight();
			int bpp = tempPixels.getBytesPerPixel();
			for (int i = 0; i < size; i++) {
				int rgbI = i * bpp;
				int rgbaI = i * 4;
				pixels[rgbaI + 0] = tempPixels[rgbI + 0];
				pixels[rgbaI + 1] = tempPixels[rgbI + 1];
				pixels[rgbaI + 2] = tempPixels[rgbI + 2];
				pixels[rgbaI + 3] = 255;
			}
		}
		else {
			pixels = image.getPixels();
		}
		currentMode = Mode::Image;
		image.setFromPixels(pixels);
		unrotatedWidth = image.getWidth();
		unrotatedHeight = image.getHeight();
		rotateImage(angle);
		pixels = image.getPixels();

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
	ofSetWindowShape(unrotatedWidth + guiWidth, unrotatedHeight);
	resetGuiPosition();
	sortingIndex = 0;
	started = false;
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
	std::string name = ((ofParameter<bool>*)sender)->getName();
	currentlySelectedThresholdVariable = sortParameterTable.find(name)->second;
	selectedThresholdVariable = (string)"Sorting by: " + name;
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
	gui.add(thresholdSlider.setup("Threshold", 0.25, 0.0, 1.0));
	gui.add(upperThresholdSlider.setup("Upper Threshold", 0.8, 0.0, 1.0));
	gui.add(angleSlider.setup("Angle", 0, 0, 359));
	gui.add(horiztonalToggleLabel.setup((std::string) "Horzontal Sort"));
	gui.add(horizontalToggle.setup("Horizontal"));
	gui.add(reverseSortLabel.setup((string)"Reverse Direction"));
	gui.add(reverseSort.setup("Reverse Sort"));
	gui.add(threadCountSlider.setup("Thread Count", 17, 0, 30));

	gui.add(selectedThresholdVariable.setup((string)"Sorting by: " + BRIGHTNESS));
	gui.add(brightnessRadio.setup(BRIGHTNESS));
	gui.add(hueRadio.setup(HUE));
	gui.add(saturationRadio.setup(SATURATION));
	brightnessRadio.addListener(this, &ofApp::selectParameterRadioButton);
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



