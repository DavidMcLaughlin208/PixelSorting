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

void pixelSortRow(int startIndex, int imageWidth, int imageHeight, ofPixels& pixelsRef, ofPixels& maskPixelsRef, ofApp::SortParameter sortParameter, float threshold, float upperThreshold, bool useMask, float currentImageAngle, int xPadding, int yPadding) {
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
		bool maskApproved = !useMask;
		if (useMask) {
			int y = int((double)i / (double)pixelsRef.getWidth());
			int x = i - (double)y * pixelsRef.getWidth();

			float rotateSine = sin(currentImageAngle * (PI / 180.0));
			float rotateCosine = cos(currentImageAngle * (PI / 180.0));

			int centerX = imageWidth / 2;
			int centerY = imageHeight / 2;

			int translatedX = x - centerX;
			int translatedY = y - centerY;

			int rotatedX = translatedX * rotateCosine - translatedY * rotateSine;
			int rotatedY = translatedX * rotateSine + translatedY * rotateCosine;

			int untranslatedX = rotatedX + centerX;
			int untranslatedY = rotatedY + centerY;

			int unpaddedX = untranslatedX - xPadding;
			int unpaddedY = untranslatedY - yPadding;


			
			if (unpaddedX < maskPixelsRef.getWidth() && unpaddedX >= 0 && unpaddedY < maskPixelsRef.getHeight() && unpaddedY >= 0) {
				maskApproved = maskPixelsRef.getColor(unpaddedX, unpaddedY).a > 0;
			}
		}

		if (maskApproved && color.a != 0 && (value >= threshold && value <= upperThreshold)) {
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
	ofEnableAlphaBlending();
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
	drawMask = maskDrawToggle;
	useMask = maskToggle;
	if (started) {
		vector<std::thread> threadList;
		for (int i = 0; i < threadCount; i++) {
			threadList.push_back(std::thread(pixelSortRow, sortingIndex, image.getWidth(), image.getHeight(), std::ref(imagePixels), std::ref(maskPixels), currentlySelectedThresholdVariable, threshold, upperThreshold, useMask, currentImageAngle, xPadding, yPadding));
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
		image.setFromPixels(imagePixels);



		if (sortComplete) {
			sortComplete = false;
			if (currentMode == Mode::Image) {
				started = false;
				
				image.setFromPixels(imagePixels);
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
					imagePixels = videoPlayer.getPixels();
					image.clear();
					image.setFromPixels(imagePixels);
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
	int bpp = imagePixels.getBytesPerPixel();
	cv::Mat src;
	src = cv::Mat_<cv::Vec4b>(image.getHeight(), image.getWidth());
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((double)i / (double)image.getWidth());
		int x = i - (double)y * image.getWidth();
		// The mismatch of indices here is because Mat is in BGRA and pixels is in RGBA format
		src.at<cv::Vec4b>(y, x)[0] = imagePixels[actualI + 2];
		src.at<cv::Vec4b>(y, x)[1] = imagePixels[actualI + 1];
		src.at<cv::Vec4b>(y, x)[2] = imagePixels[actualI + 0];
		src.at<cv::Vec4b>(y, x)[3] = imagePixels[actualI + 3];
	}
	
	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle, center not relevant
	cv::Size boxSize;
	if (paddingAddedToImage) {
		boxSize = src.size();
	}
	else {
		int diagonal = (int)sqrt(src.cols * src.cols + src.rows * src.rows);
		boxSize = cv::Size(diagonal, diagonal);
		this->xPadding = (boxSize.width - src.size().width) / 2;
		this->yPadding = (boxSize.height - src.size().height) / 2;
	}
	// adjust transformation matrix
	rot.at<double>(0, 2) += boxSize.width / 2.0 - src.cols / 2.0;
	rot.at<double>(1, 2) += boxSize.height / 2.0 - src.rows / 2.0;

	cv::Mat dst;
	cv::warpAffine(src, dst, rot, boxSize);


	image.resize(dst.cols, dst.rows);
	size = image.getWidth() * image.getHeight();
	imagePixels.allocate(dst.cols, dst.rows, OF_IMAGE_COLOR_ALPHA);
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((double)i / (double)image.getWidth());
		int x = i - (double) y * image.getWidth();
		imagePixels[actualI + 2] = dst.at<cv::Vec4b>(y, x)[0];
		imagePixels[actualI + 1] = dst.at<cv::Vec4b>(y, x)[1];
		imagePixels[actualI + 0] = dst.at<cv::Vec4b>(y, x)[2];
		imagePixels[actualI + 3] = dst.at<cv::Vec4b>(y, x)[3];
	}
	image.setFromPixels(imagePixels);
	imagePixels = image.getPixels();
}

void ofApp::rotateMask(int angle, bool paddingAddedToMask) {
	if (angle == 0) {
		return;
	}
	int size = maskCopyPixels.getWidth() * maskCopyPixels.getHeight();
	int bpp = maskCopyPixels.getBytesPerPixel();
	cv::Mat src;
	src = cv::Mat_<cv::Vec4b>(maskCopyPixels.getHeight(), maskCopyPixels.getWidth());
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((double)i / (double)maskCopyPixels.getWidth());
		int x = i - (double)y * maskCopyPixels.getWidth();
		// The mismatch of indices here is because Mat is in BGRA and pixels is in RGBA format
		src.at<cv::Vec4b>(y, x)[0] = maskCopyPixels[actualI + 2];
		src.at<cv::Vec4b>(y, x)[1] = maskCopyPixels[actualI + 1];
		src.at<cv::Vec4b>(y, x)[2] = maskCopyPixels[actualI + 0];
		src.at<cv::Vec4b>(y, x)[3] = maskCopyPixels[actualI + 3];
	}

	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle, center not relevant
	cv::Size boxSize;
	if (paddingAddedToMask) {
		boxSize = src.size();
	}
	else {
		int diagonal = (int)sqrt(src.cols * src.cols + src.rows * src.rows);
		boxSize = cv::Size(diagonal, diagonal);
	}
	// adjust transformation matrix
	rot.at<double>(0, 2) += boxSize.width / 2.0 - src.cols / 2.0;
	rot.at<double>(1, 2) += boxSize.height / 2.0 - src.rows / 2.0;

	cv::Mat dst;
	cv::warpAffine(src, dst, rot, boxSize);


	maskCopy.resize(dst.cols, dst.rows);
	size = image.getWidth() * image.getHeight();
	maskCopyPixels.allocate(dst.cols, dst.rows, OF_IMAGE_COLOR_ALPHA);
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((double)i / (double)image.getWidth());
		int x = i - (double)y * image.getWidth();
		maskCopyPixels[actualI + 2] = dst.at<cv::Vec4b>(y, x)[0];
		maskCopyPixels[actualI + 1] = dst.at<cv::Vec4b>(y, x)[1];
		maskCopyPixels[actualI + 0] = dst.at<cv::Vec4b>(y, x)[2];
		maskCopyPixels[actualI + 3] = dst.at<cv::Vec4b>(y, x)[3];
	}
	maskCopy.setFromPixels(maskCopyPixels);
	maskCopyPixels = maskCopy.getPixels();

}

void ofApp::saveFrameToVideo() {
	int size = image.getWidth() * image.getHeight();
	int bpp = imagePixels.getBytesPerPixel();
	cv::Mat mat;
	mat = cv::Mat_<cv::Vec3b>(image.getHeight(), image.getWidth());
	for (int i = 0; i < size; i++) {
		int actualI = i * bpp;
		int y = int((float)i / (float)image.getWidth());
		int x = i - y * image.getWidth();
		mat.at<cv::Vec3b>(y, x)[0] = imagePixels[actualI + 2];
		mat.at<cv::Vec3b>(y, x)[1] = imagePixels[actualI + 1];
		mat.at<cv::Vec3b>(y, x)[2] = imagePixels[actualI + 0];
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
		ofSetColor(255, 255, 255, 255);
		ofPushMatrix();
		ofTranslate(unrotatedWidth / 2, unrotatedHeight / 2 );
		ofRotate(currentImageAngle, 0, 0, 1);
		ofTranslate(-image.getWidth() / 2, -image.getHeight() / 2);
		ofPushMatrix();
		image.draw(0,0, wid , hei);
		ofPopMatrix();
		ofPopMatrix();


		
	}
	if (mask.isAllocated() && drawMask) {
		ofPushMatrix();
		ofSetColor(255, 255, 255, 100);
		mask.draw(0, 0);
		ofPopMatrix();
	}
	gui.draw();
	maskPanel.draw();
}

void ofApp::loadMask(std::string fileName) {
	ofFilePath filePath;
	std::string extension = "." + filePath.getFileExt(fileName);
	if (imageExtensions.find(extension) != imageExtensions.end()) {
		mask.clear();
		mask.load("images/masks/" + fileName);
		mask.setImageType(OF_IMAGE_COLOR_ALPHA);

		mask.getPixels().setChannel(3, mask.getPixels().getChannel(0));
		mask.update();
		maskPixels = mask.getPixels();
		paddingAddedToMask = false;

		maskCopyPixels.clear();
		maskCopyPixels.allocate(unrotatedWidth, unrotatedHeight, OF_IMAGE_COLOR_ALPHA);
		maskCopyPixels.setColor(ofColor(0, 0, 0, 0));
		maskPixels.pasteInto(maskCopyPixels, 0, 0);
		maskPixels.clear();
		maskPixels.allocate(unrotatedWidth, unrotatedHeight, OF_IMAGE_COLOR_ALPHA);
		maskCopyPixels.pasteInto(maskPixels, 0, 0);
		mask.update();
		maskPixels = mask.getPixels();
		maskCurrentAngle = 0;

		if (currentImageAngle != 0) {
			rotateMask(currentImageAngle, paddingAddedToMask);
			paddingAddedToMask = true;
			maskCurrentAngle = currentImageAngle;
		}
	}
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
		imagePixels = image.getPixels();
		paddingAddedToImage = false;
		currentImageAngle = 0;
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
		imagePixels = videoPlayer.getPixels();
		image.setFromPixels(imagePixels);

		float fps = videoPlayer.getTotalNumFrames() / videoPlayer.getDuration();
		videoWriter = cv::VideoWriter("data/images/effect.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(image.getWidth(), image.getHeight()), true);
		currentMode = Mode::Video;
	}
	else {
		//Display some error message
		currentMode = Mode::None;
		return;
	}
	if (mask.isAllocated()) {
		maskCopyPixels.clear();
		maskCopyPixels.allocate(unrotatedWidth, unrotatedHeight, OF_IMAGE_COLOR_ALPHA);
		maskCopyPixels.setColor(ofColor(0, 0, 0, 0));
		maskPixels.pasteInto(maskCopyPixels, 0, 0);
		maskPixels.clear();
		maskPixels.allocate(unrotatedWidth, unrotatedHeight, OF_IMAGE_COLOR_ALPHA);
		maskCopyPixels.pasteInto(maskPixels, 0, 0);
		mask.update();
		maskPixels = mask.getPixels();
		maskCurrentAngle = 0;
		paddingAddedToMask = false;
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
			currentImageAngle = angle;

			/*if (useMask && mask.isAllocated()) {
				rotateMask(currentImageAngle - maskCurrentAngle, paddingAddedToMask);
				paddingAddedToMask = true;
				maskCurrentAngle = angle;
			}*/
		}	
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

bool ofApp::clickedOnImageButton(const void* sender) {
	ofParameter<bool>* button = (ofParameter<bool>*)sender;
	loadImage(button->getName());
	return true;
}

void ofApp::resetGuiPosition() {
	gui.setPosition(ofGetWidth() - guiWidth, 10);
	maskPanel.setPosition(ofGetWidth() - guiWidth * 2 - 10, 10);
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
		button->addListener(this, &ofApp::clickedOnImageButton);
	}

	maskPanel.setup();
	maskPanel.setPosition(ofGetWidth() - guiWidth * 2 - 10, 10);
	maskPanel.add(maskToggle.setup(false));
	maskPanel.add(maskDrawToggle.setup(false));
	directory.open("images/masks");
	directory.listDir();
	for (int i = 0; i < directory.size(); i++) {
		ofxButton* button = new ofxButton();
		maskPanel.add(button->setup(directory.getName(i)));
		maskFileButtons.push_back(button);
		button->addListener(this, &ofApp::clickOnMaskImageButton);
	}
}

void ofApp::saveCurrentImage() {
	if (image.isAllocated()) {
		ofFilePath filePath;
		std::string fileName = filePath.getBaseName(currentFileName);
		std::string extension = "." + filePath.getFileExt(currentFileName);
		std::string fullName = fileName + "1" + extension;
		ofPixels copy;
		copy.allocate(imagePixels.getWidth(), imagePixels.getHeight(), imagePixels.getImageType());
		imagePixels.pasteInto(copy, 0, 0);
		rotateImage(-currentImageAngle, paddingAddedToImage);
		int originalImageX = image.getWidth() / 2 - unrotatedWidth / 2;
		int originalImageY = image.getHeight() / 2 - unrotatedHeight / 2;
		imagePixels.crop(originalImageX + 1, originalImageY + 1, unrotatedWidth, unrotatedHeight);
		image.resize(imagePixels.getWidth(), imagePixels.getHeight());
		image.setFromPixels(imagePixels);
		image.save("images/" + fullName);
		currentFileName = fullName;
		//pixels.allocate(copy.getWidth(), copy.getHeight(), OF_IMAGE_COLOR_ALPHA);
		//copy.pasteInto(pixels, 0, 0);
		imagePixels = copy;
		image.setFromPixels(imagePixels);
	}
	
}

bool ofApp::clickOnMaskImageButton(const void* sender) {
	ofParameter<bool>* button = (ofParameter<bool>*)sender;
	loadMask(button->getName());
	return true;
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



