#include "ofApp.h"

std::string ofApp::BRIGHTNESS = "Brightness";
std::string ofApp::LIGHTNESS = "Lightness";
std::string ofApp::HUE = "Hue";
std::string ofApp::SATURATION = "Saturation";
std::string ofApp::USEMASKTITLE = "Use Mask";
std::string ofApp::MASKOPACITYTITLE = "Mask Opacity";
std::string ofApp::DRAWMASKTOOLTITLE = "Draw Mask Tool";
std::string ofApp::BRUSHSIZESLIDERTITLE = "Brush Size";
std::string ofApp::CIRCLE = "Circle";
std::string ofApp::SQUARE = "Square";
std::string ofApp::CLICKANDDRAG = "Click And Drag";
std::string ofApp::SORTBUTTONTITLE = "Sort";
std::string ofApp::SAVEIMAGEBUTTONTITLE = "Save Image";
std::string ofApp::LOWERTHRESHOLDTITLE = "Lower Threshold";
std::string ofApp::UPPERTHRESHOLDTITLE = "Upper Threshold";
std::string ofApp::ANGLESLIDERTITLE = "Angle";
std::string ofApp::THREADCOUNTSLIDERTITLE = "Thread Count";

void transferFromPixelsToMat(ofPixels& pixelsRef, cv::Mat& matRef, int section, int sectionLength, bool endingSection, int width, int height) {
	int bpp = pixelsRef.getBytesPerPixel();
	int start = section * sectionLength;
	int end = endingSection ? width * height : (section + 1) * sectionLength;
	for (int i = start; i < end; i++) {
		int actualI = i * bpp;
		int y = int((double)i / (double)width);
		int x = i - (double)y * width;
		// The mismatch of indices here is because Mat is in BGRA and pixels is in RGBA format
		matRef.at<cv::Vec4b>(y, x)[0] = pixelsRef[actualI + 2];
		matRef.at<cv::Vec4b>(y, x)[1] = pixelsRef[actualI + 1];
		matRef.at<cv::Vec4b>(y, x)[2] = pixelsRef[actualI + 0];
		matRef.at<cv::Vec4b>(y, x)[3] = pixelsRef[actualI + 3];
	}
}

void transferFromMatToPixels(ofPixels& pixelsRef, cv::Mat& matRef, int section, int sectionLength, bool endingSection, int width, int height) {
	int bpp = pixelsRef.getBytesPerPixel();
	int start = section * sectionLength;
	int end = endingSection ? width * height : (section + 1) * sectionLength;
	for (int i = start; i < end; i++) {
		int actualI = i * bpp;
		int y = int((double)i / (double)width);
		int x = i - (double)y * width;
		// The mismatch of indices here is because Mat is in BGRA and pixels is in RGBA format
		pixelsRef[actualI + 2] = matRef.at<cv::Vec4b>(y, x)[0];
		pixelsRef[actualI + 1] = matRef.at<cv::Vec4b>(y, x)[1];
		pixelsRef[actualI + 0] = matRef.at<cv::Vec4b>(y, x)[2];
		pixelsRef[actualI + 3] = matRef.at<cv::Vec4b>(y, x)[3];
	}
}

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
			// Take rotated pixel location in image and find its unrotated location to check in the mask pixels
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

	brushTypeOptions = { CIRCLE, SQUARE, CLICKANDDRAG };
	sortingParameterOptions = { BRIGHTNESS, HUE, SATURATION };
	setupDatGui();

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
	threshold = thresholdSlider->getValue();
	upperThreshold = upperThresholdSlider->getValue();
	angle = angleSlider->getValue();
	threadCount = threadCountSlider->getValue();
	maskOpacity = maskOpacitySlider->getValue() * 255;
	brushSize = brushSizeSlider->getValue();
	useMask = useMaskToggle->getChecked();
	if (started) {
		vector<std::thread> threadList;
		for (int i = 0; i < threadCount; i++) {
			threadList.push_back(std::thread(pixelSortRow, sortingIndex, image.getWidth(), image.getHeight(), std::ref(imagePixels), std::ref(maskPixels), currentlySelectedSortParameter, threshold, upperThreshold, useMask, currentImageAngle, xPadding, yPadding));
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
				if (videoPlayer.getCurrentFrame() > videoPlayer.getTotalNumFrames() - 1) {
					videoPlayer.close();
					videoWriter.release();
					started = false;
					std::cout << "Completed sorting video" << std::endl;
				}
				else {
					videoPlayer.nextFrame();
					imagePixels = videoPlayer.getPixels();
					imagePixels.setImageType(OF_IMAGE_COLOR_ALPHA);
					image.clear();
					image.setFromPixels(imagePixels);
					image.setImageType(OF_IMAGE_COLOR_ALPHA);
					image.update();
					currentImageAngle = 0;
					if (currentImageAngle != angle) {
						rotateImage(angle, false);
						paddingAddedToImage = true;
						currentImageAngle = angle;
					}
					std::cout << "Starting frame " << videoPlayer.getCurrentFrame() << " out of " << videoPlayer.getTotalNumFrames() << std::endl;
					timeStart = std::chrono::high_resolution_clock::now();
				}
			}
		}

		
	}
	//maskImagesScrollView->setPosition(ofGetWidth() - guiWidth * 2, datMaskPanel->getHeight() + 10);
	imageScrollView->update();
	maskImagesScrollView->update();
}

// Code adapted to work with OF pulled from here: https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c/33564950#33564950
void ofApp::rotateImage(int angle, bool paddingAddedToImage) {
	std::chrono::steady_clock::time_point rotateStart = std::chrono::high_resolution_clock::now();

	if (angle == 0) {
		return;
	}
	int size = image.getWidth() * image.getHeight();
	int bpp = imagePixels.getBytesPerPixel();
	cv::Mat src;
	src = cv::Mat_<cv::Vec4b>(image.getHeight(), image.getWidth());

	// Multithread on transferring pixels from OF data structure to OpenCV data structure
	// This is a waste of time regardless. Could potentially rewrite all of the sorting and image logic to just use OpenCV
	// to eliminate this entirely
	vector<std::thread> threadList;
	for (int i = 0; i < pixelTransferThreadCount; i++) {
		threadList.push_back(std::thread(transferFromPixelsToMat, std::ref(imagePixels), std::ref(src), i, size / pixelTransferThreadCount, i == pixelTransferThreadCount - 1, image.getWidth(), image.getHeight()));
	}
	for (int i = 0; i < threadList.size(); i++) {
		threadList[i].join();
	}
	
	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
	// determine bounding rectangle, center not relevant
	// We calculate the largest bounding box necessary for the image to fit at any angle so if multiple rotations
	// occur on the same image we only have to add padding once
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
	cv::warpAffine(src, dst, rot, boxSize, cv::INTER_CUBIC);


	image.resize(dst.cols, dst.rows);
	size = image.getWidth() * image.getHeight();
	imagePixels.allocate(dst.cols, dst.rows, OF_IMAGE_COLOR_ALPHA);
	vector<std::thread> threadList2;
	for (int i = 0; i < pixelTransferThreadCount; i++) {
		threadList2.push_back(std::thread(transferFromMatToPixels, std::ref(imagePixels), std::ref(dst), i, size / pixelTransferThreadCount, i == pixelTransferThreadCount - 1, image.getWidth(), image.getHeight()));
	}
	for (int i = 0; i < threadList2.size(); i++) {
		threadList2[i].join();
	}
	image.setFromPixels(imagePixels);
	imagePixels = image.getPixels();
	std::chrono::steady_clock::time_point rotateEnd = std::chrono::high_resolution_clock::now();
	long timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(rotateEnd - rotateStart).count();
	std::cout << "Rotating image took " << timeTaken << " milliseconds for rotation diff of " << angle << " degrees." << std::endl;
}

void ofApp::saveFrameToVideo() {
	if (currentImageAngle != 0) {
		ofPixels copy;
		copy.allocate(imagePixels.getWidth(), imagePixels.getHeight(), imagePixels.getImageType());
		imagePixels.pasteInto(copy, 0, 0);
		rotateImage(-currentImageAngle, true);
		int originalImageX = image.getWidth() / 2 - unrotatedWidth / 2;
		int originalImageY = image.getHeight() / 2 - unrotatedHeight / 2;
		imagePixels.crop(originalImageX, originalImageY, unrotatedWidth, unrotatedHeight);
		image.resize(imagePixels.getWidth(), imagePixels.getHeight());
		image.setFromPixels(imagePixels);
	}
	int size = imagePixels.getWidth() * imagePixels.getHeight();
	int bpp = imagePixels.getBytesPerPixel();
	cv::Mat mat;
	mat = cv::Mat_<cv::Vec3b>(imagePixels.getHeight(), imagePixels.getWidth());
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

//--------------------------------------------------------------
void ofApp::draw() {
	if (image.isAllocated()) {
		int wid = unrotatedWidth * currentRatio;
		int hei = unrotatedHeight * currentRatio;
		imageFbo.begin();
		ofSetColor(255, 255, 255, 255);
		ofPushMatrix();
		ofTranslate(unrotatedWidth / 2, unrotatedHeight / 2 );
		ofRotate(currentImageAngle, 0, 0, 1);
		ofTranslate(-image.getWidth() / 2, -image.getHeight() / 2);
		ofPushMatrix();
		image.draw(0,0);
		ofPopMatrix();
		ofPopMatrix();
		imageFbo.end();

		ofSetColor(255, 255, 255, 255);
		imageFbo.draw(0, 0, wid, hei);
	}
	if (mask.isAllocated()) {
		ofPushMatrix();
		ofSetColor(255, 255, 255, maskOpacity);
		mask.draw(0, 0, mask.getWidth() * currentRatio, mask.getHeight() * currentRatio);
		ofPopMatrix();
	}
	if (currentMouseMode == MouseMode::MaskDraw && withinUnrotatedImageBounds(mouseX, mouseY)) {
		switch (currentBrushMode) {
		case (BrushMode::Circle):
			ofNoFill();
			ofSetColor(255, 0, 0, 255);
			ofCircle(glm::vec3(mouseX, mouseY, 0), brushSize);
			break;
		case (BrushMode::Square):
			ofNoFill();
			ofSetColor(255, 0, 0, 255);
			ofRect(mouseX - brushSize, mouseY - brushSize, brushSize * 2, brushSize * 2);
		}
	}
	imageScrollView->draw();
	maskImagesScrollView->draw();
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
		if (mask.getWidth() < unrotatedWidth || mask.getHeight() < unrotatedHeight) {
			ofPixels copy;
			copy.allocate(maskPixels.getWidth(), maskPixels.getHeight(), OF_IMAGE_COLOR_ALPHA);
			maskPixels.pasteInto(copy, 0, 0);
			mask.allocate(max((int)mask.getWidth(), unrotatedWidth), max((int)mask.getHeight(), unrotatedHeight), OF_IMAGE_COLOR_ALPHA);
			mask.setColor(ofColor(0, 0, 0, 0));
			maskPixels = mask.getPixels();
			copy.pasteInto(maskPixels, 0, 0);
			mask.setFromPixels(maskPixels);
			maskPixels = mask.getPixels();
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
		if (!videoPlayer.isLoaded()) {
			return;
		}
		videoPlayer.firstFrame();
		videoPlayer.play();
		videoPlayer.setPaused(true);
		imagePixels = videoPlayer.getPixels();
		imagePixels.setImageType(OF_IMAGE_COLOR_ALPHA);
		image.setFromPixels(imagePixels);
		image.setImageType(OF_IMAGE_COLOR_ALPHA);
		image.update();
		unrotatedWidth = image.getWidth();
		unrotatedHeight = image.getHeight();
		paddingAddedToImage = false;
		currentImageAngle = 0;

		float fps = videoPlayer.getTotalNumFrames() / videoPlayer.getDuration();
		videoWriter = cv::VideoWriter("data/images/effect.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(image.getWidth(), image.getHeight()), true);
		currentMode = Mode::Video;
	}
	else {
		// Display some error message
		currentMode = Mode::None;
		return;
	}

	imageFbo.clear();
	imageFbo.allocate(unrotatedWidth, unrotatedHeight);
	imageFbo.begin();
	ofClear(255, 255, 255, 0);
	imageFbo.end();

	int wid = unrotatedWidth;
	int hei = unrotatedHeight;
	if (wid > maxWidth || hei > maxHeight) {
		float xRatio = (float)maxWidth / (float)wid;
		float yRatio = (float)maxHeight / (float)hei;
		currentRatio = xRatio < yRatio ? xRatio : yRatio;
	}
	else {
		currentRatio = 1.0f;
	}

	if (mask.isAllocated()) {
		if (mask.getWidth() < unrotatedWidth || mask.getHeight() < unrotatedHeight) {
			ofPixels copy;
			copy.allocate(maskPixels.getWidth(), maskPixels.getHeight(), OF_IMAGE_COLOR_ALPHA);
			maskPixels.pasteInto(copy, 0, 0);
			mask.allocate(max((int)mask.getWidth(), unrotatedWidth), max((int)mask.getHeight(), unrotatedHeight), OF_IMAGE_COLOR_ALPHA);
			mask.setColor(ofColor(0, 0, 0, 0));
			maskPixels = mask.getPixels();
			copy.pasteInto(maskPixels, 0, 0);
			mask.setFromPixels(maskPixels);
			maskPixels = mask.getPixels();
		}
	}
	else {
		mask.allocate(unrotatedWidth, unrotatedHeight, OF_IMAGE_COLOR_ALPHA);
		mask.setColor(ofColor(0, 0, 0, 0));
		mask.update();
		maskPixels = mask.getPixels();
	}
	currentFileName = fileName;
	ofSetWindowShape(unrotatedWidth * currentRatio + guiWidth * 2, unrotatedHeight * currentRatio);
	resetGuiPosition();
	sortingIndex = 0;
	started = false;
}

void ofApp::start(ofxDatGuiButtonEvent e) {
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
		}	
	}
	else {
		videoWriter.release();
	}
}

void ofApp::selectParameterRadioButton(const void* sender) {
	std::string name = ((ofParameter<bool>*)sender)->getName();
	currentlySelectedSortParameter = sortParameterTable.find(name)->second;
	selectedThresholdVariable = (string)"Sorting by: " + name;
}

bool ofApp::clickedOnImageButton(const void* sender) {
	ofParameter<bool>* button = (ofParameter<bool>*)sender;
	loadImage(button->getName());
	return true;
}

void ofApp::resetGuiPosition() {
	datImagePanel->setPosition(ofGetWidth() - guiWidth * 2, 10);
	imageScrollView->setPosition(ofGetWidth() - guiWidth * 2, datImagePanel->getHeight() + 10);
	datMaskPanel->setPosition(ofGetWidth() - guiWidth, 10);
	maskImagesScrollView->setPosition(ofGetWidth() - guiWidth, datMaskPanel->getHeight() + 10);
}

void ofApp::setupDatGui() {
	datImagePanel = new ofxDatGui(ofGetWidth() - guiWidth * 2, 10);
	datImagePanel->setWidth(guiWidth);
	datImagePanel->addHeader("Image Sorting Controls");
	ofxDatGuiButton* sortButton = datImagePanel->addButton(SORTBUTTONTITLE);
	sortButton->onButtonEvent(this, &ofApp::start);
	ofxDatGuiButton* saveButton = datImagePanel->addButton(SAVEIMAGEBUTTONTITLE);
	saveButton->onButtonEvent(this, &ofApp::saveCurrentImage);
	ofxDatGuiDropdown* sortingParameterDropdown = datImagePanel->addDropdown("Sorting Parameter", sortingParameterOptions);
	sortingParameterDropdown->onDropdownEvent(this, &ofApp::selectSortingParameter);
	sortingParameterDropdown->select(0);
	thresholdSlider = datImagePanel->addSlider(LOWERTHRESHOLDTITLE, 0.0f, 1.0f, 0.25f);
	upperThresholdSlider = datImagePanel->addSlider(UPPERTHRESHOLDTITLE, 0.0f, 1.0f, 0.80f);
	angleSlider = datImagePanel->addSlider(ANGLESLIDERTITLE, 0, 359, 0);
	angleSlider->setPrecision(0);
	threadCountSlider = datImagePanel->addSlider(THREADCOUNTSLIDERTITLE, 1, 30, 17);
	datImagePanel->addLabel("Load Image");
	imageScrollView = new ofxDatGuiScrollView("Mask Image Files", (ofGetHeight() - datImagePanel->getHeight()) / 26 - 1);
	imageScrollView->setWidth(guiWidth);
	imageScrollView->setPosition(ofGetWidth() - guiWidth * 2, datImagePanel->getHeight() + 10);
	imageScrollView->onScrollViewEvent(this, &ofApp::clickOnImageButton);
	imageDirectory.open("images");
	imageDirectory.listDir();
	for (int i = 0; i < imageDirectory.size(); i++) {
		imageScrollView->add(imageDirectory.getName(i));
		imageScrollView->getItemAtIndex(i)->setLabelUpperCase(false);
	}

	datMaskPanel = new ofxDatGui(ofGetWidth() - guiWidth, 10);
	datMaskPanel->setWidth(guiWidth);
	datMaskPanel->addHeader("Mask Controls");
	useMaskToggle = datMaskPanel->addToggle(USEMASKTITLE);
	maskOpacitySlider = datMaskPanel->addSlider(MASKOPACITYTITLE, 0.0, 1.0, 0.4);
	ofxDatGuiButton* maskBrushToggle = datMaskPanel->addButton(DRAWMASKTOOLTITLE);
	maskBrushToggle->onButtonEvent(this, &ofApp::maskToolToggleClicked);
	ofxDatGuiDropdown* brushTypeDropdown = datMaskPanel->addDropdown("Brush Type", brushTypeOptions);
	brushTypeDropdown->onDropdownEvent(this, &ofApp::brushTypeSelected);
	brushTypeDropdown->select(0);
	brushSizeSlider = datMaskPanel->addSlider(BRUSHSIZESLIDERTITLE, 0, 100, 10);
	datMaskPanel->addLabel("Load Mask");
	maskImagesScrollView = new ofxDatGuiScrollView("Mask Image Files", (ofGetHeight() - datMaskPanel->getHeight()) / 26 - 1);
	maskImagesScrollView->setWidth(guiWidth);
	maskImagesScrollView->setPosition(ofGetWidth() - guiWidth, datMaskPanel->getHeight() + 10);
	maskImagesScrollView->onScrollViewEvent(this, &ofApp::clickOnMaskImageButton);
	maskDirectory.open("images/masks");
	maskDirectory.listDir();
	for (int i = 0; i < maskDirectory.size(); i++) {
		maskImagesScrollView->add(maskDirectory.getName(i));
		maskImagesScrollView->getItemAtIndex(i)->setLabelUpperCase(false);
	}
	
}

void ofApp::saveCurrentImage(ofxDatGuiButtonEvent e) {
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
		imagePixels.crop(originalImageX, originalImageY, unrotatedWidth, unrotatedHeight);
		image.resize(imagePixels.getWidth(), imagePixels.getHeight());
		image.setFromPixels(imagePixels);
		image.save("images/" + fullName);
		currentFileName = fullName;
		imagePixels = copy;
		image.setFromPixels(imagePixels);
	}
	
}

void ofApp::clickOnMaskImageButton(ofxDatGuiScrollViewEvent e) {
	loadMask(e.target->getName());
}

void ofApp::clickOnImageButton(ofxDatGuiScrollViewEvent e) {
	loadImage(e.target->getName());
}

void ofApp::maskToolToggleClicked(ofxDatGuiButtonEvent e) {
	if (currentMouseMode != MouseMode::MaskDraw) {
		currentMouseMode = MouseMode::MaskDraw;
	}
	else {
		currentMouseMode = MouseMode::Default;
	}
}

void ofApp::brushTypeSelected(ofxDatGuiDropdownEvent e) {
	currentBrushMode = (BrushMode)e.child;
}

void ofApp::selectSortingParameter(ofxDatGuiDropdownEvent e) {
	currentlySelectedSortParameter = (SortParameter)e.child;
}

void ofApp::applyBrushStroke(int centerX, int centerY, int size, ofApp::BrushMode mode, int value) {
	int scaledCenterX = centerX / currentRatio;
	int scaledCenterY = centerY / currentRatio;
	int scaledSize = size / currentRatio;
	int topLeftX = scaledCenterX - scaledSize;
	int topLeftY = scaledCenterY - scaledSize;
	for (int y = 0; y < scaledSize * 2; y++) {
		for (int x = 0; x < scaledSize * 2; x++) {
			int modX = topLeftX + x;
			int modY = topLeftY + y;
			if (modX >= 0 && modX < mask.getWidth() && modY >= 0 && modY < mask.getHeight()) {
				if (mode == BrushMode::Circle) {
					float distance = sqrt(pow(modX - scaledCenterX, 2) + pow(modY - scaledCenterY, 2));
					if (distance > (float) scaledSize) continue;
				}
				if (maskPixels.getColor(modX, modY).a != value) {
					maskPixels.setColor(modX, modY, ofColor(value, value, value, value));
				}
			}
		}
	}
	mask.setFromPixels(maskPixels);
	maskPixels = mask.getPixels();
}

bool ofApp::withinMaskBounds(int x, int y) {
	return x >= 0 && x < mask.getWidth() && y >= 0 && y < mask.getHeight();
}

bool ofApp::withinUnrotatedImageBounds(int x, int y) {
	return x >= 0 && x < unrotatedWidth * currentRatio && y >= 0 && y < unrotatedHeight * currentRatio;
}


bool ofApp::cycleBrushMode() {
	if (currentBrushMode == BrushMode::Circle) {
		currentBrushMode = BrushMode::Square;
	}
	else if (currentBrushMode == BrushMode::Square) {
		currentBrushMode = BrushMode::Circle;
	}
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
	if (currentMouseMode == MouseMode::MaskDraw && withinMaskBounds(x, y)) {
		applyBrushStroke(x, y, brushSize, currentBrushMode, 255 * (button / 2));
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (currentMouseMode == MouseMode::MaskDraw && withinMaskBounds(x, y)) {
		applyBrushStroke(x, y, brushSize, currentBrushMode, 255 * (button / 2));
	}
}

void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
	if (!withinUnrotatedImageBounds(x, y)) {
		return;
	}
	if (scrollY > 0) {
		brushSize = min(100, brushSize + 1);
	}
	else {
		brushSize = max(0, brushSize - 1);
	}
	//maskBrushSizeSlider = brushSize;
	datMaskPanel->getSlider(BRUSHSIZESLIDERTITLE)->setValue(brushSize);
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



