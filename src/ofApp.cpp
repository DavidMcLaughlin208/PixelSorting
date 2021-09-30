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

void pixelSortRow(int startIndex, int imageWidth, int imageHeight, ofPixels& pixelsRef, ofApp::SortParameter sortParameter, float threshold, float upperThreshold) {
	int start = startIndex;
	int end = (start + 1) * imageWidth;
	int columnsOrRows = imageHeight;
	float highestVal;
	int indexOfHighest;

	int bytesPerPixel = pixelsRef.getBytesPerPixel();

	int startOfInterval = -1;
	int endOfInterval = -1;

	ofColor color;
	char pixelSwapBuffer[4];
	for (int i = start * imageWidth; i < end; i++) {
		int actualI = i * bytesPerPixel;
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
			int actualS = s * bytesPerPixel;
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
			int actualS = s * bytesPerPixel;
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
	imageExtensions.insert(".jpeg");

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
	threadCount = threadCountSlider;
	if (started) {
		vector<std::thread> threadList;
		for (int i = 0; i < threadCount; i++) {
			threadList.push_back(std::thread(pixelSortRow, sortingIndex, image.getWidth(), image.getHeight(), std::ref(pixels), currentlySelectedThresholdVariable, threshold, upperThreshold));
			sortingIndex += 1;

			if (sortingIndex >= image.getHeight() - 1) {
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
				
				image.setFromPixels(pixels);
				//currentImageAngle = 0;
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
void ofApp::rotateImage(int angle, bool paddingAddedToImage) {
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
		// The mismatch of indices here is because Mat is in BGRA and pixels is in RGBA format
		src.at<cv::Vec4b>(y, x)[0] = pixels[actualI + 2];
		src.at<cv::Vec4b>(y, x)[1] = pixels[actualI + 1];
		src.at<cv::Vec4b>(y, x)[2] = pixels[actualI + 0];
		src.at<cv::Vec4b>(y, x)[3] = pixels[actualI + 3];
	}
	
	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle, center not relevant
	cv::Size boxSize;
	if (!paddingAddedToImage) {
		cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), 45).boundingRect2f();
		// adjust transformation matrix
		boxSize = bbox.size();
	}
	else {
		boxSize = src.size();
	}
	rot.at<double>(0, 2) += boxSize.width / 2.0 - src.cols / 2.0;
	rot.at<double>(1, 2) += boxSize.height / 2.0 - src.rows / 2.0;

	cv::Mat dst;
	cv::warpAffine(src, dst, rot, boxSize);


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
		//if (started) {
			ofTranslate(image.getWidth() / 2, image.getHeight() / 2);
			ofRotate(currentImageAngle);
			ofTranslate(-image.getWidth() / 2.5, -image.getHeight() / 2.5);
			image.draw(0, 0);// , wid, hei);
			ofTranslate(image.getWidth() / 2.5, image.getHeight() / 2.5);
			ofRotate(-currentImageAngle);
			ofTranslate(-image.getWidth() / 2, -image.getHeight() / 2);
		//}
		//else {
		//	image.draw(0, 0);
		//}
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
		image.setImageType(OF_IMAGE_COLOR_ALPHA);
		currentMode = Mode::Image;
		unrotatedWidth = image.getWidth();
		unrotatedHeight = image.getHeight();
		pixels = image.getPixels();
		paddingAddedToImage = false;
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
	if (currentMode == Mode::None) {
		return;
	}
	started = !started;
	if (started) {
		timeStart = std::chrono::high_resolution_clock::now();
		if (angle != currentImageAngle) {
			rotateImage(angle - currentImageAngle, paddingAddedToImage);
			paddingAddedToImage = true;
		}

		currentImageAngle = angle;
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
		ofPixels copy;
		copy.allocate(pixels.getWidth(), pixels.getHeight(), pixels.getImageType());
		pixels.pasteInto(copy, 0, 0);
		rotateImage(-currentImageAngle, paddingAddedToImage);
		int originalImageX = image.getWidth() / 2 - unrotatedWidth / 2;
		int originalImageY = image.getHeight() / 2 - unrotatedHeight / 2;
		pixels.crop(originalImageX + 1, originalImageY + 1, unrotatedWidth, unrotatedHeight);
		image.resize(pixels.getWidth(), pixels.getHeight());
		image.setFromPixels(pixels);
		image.save("images/" + fullName);
		currentFileName = fullName;
		pixels.allocate(copy.getWidth(), copy.getHeight(), OF_IMAGE_COLOR_ALPHA);
		copy.pasteInto(pixels, 0, 0);
		image.setFromPixels(pixels);
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



