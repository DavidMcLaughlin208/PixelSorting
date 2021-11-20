#include "ofApp.h"

std::string ofApp::BRIGHTNESS = "Brightness";
std::string ofApp::LIGHTNESS = "Lightness";
std::string ofApp::HUE = "Hue";
std::string ofApp::SATURATION = "Saturation";
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
std::string ofApp::IDLE = "Idle";

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

void pixelSortRow(int startIndex, int imageWidth, int imageHeight, ofPixels& pixelsRef, ofPixels& maskPixelsRef, ofApp::SortParameter sortParameter, float threshold, float upperThreshold, bool useMask, int maskThreshold, float currentImageAngle, int xPadding, int yPadding) {
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
				maskApproved = maskPixelsRef.getColor(unpaddedX, unpaddedY).a >= maskThreshold;
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

		// TODO: Instead of copying pixels to a vector and then soritng and putting them back. 
		// Look into extending ofPixels or some way to sort the pixels in place
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
	ofSetEscapeQuitsApp(false);

	brushTypeOptions = { CIRCLE, SQUARE, CLICKANDDRAG };
	sortingParameterOptions = { BRIGHTNESS, HUE, SATURATION };
	
	// Video codecs included in K-Lite codec pack for windows: https://codecguide.com/download_kl.htm
	// AVI, MKV, MP4, FLV, MPEG, MOV, TS, M2TS, WMV, RM, RMVB, OGM, WebM
	videoExtensions.push_back("avi");
	videoExtensions.push_back("mkv");
	videoExtensions.push_back("mp4");
	videoExtensions.push_back("flv");
	videoExtensions.push_back("mpeg");
	videoExtensions.push_back("mov");
	videoExtensions.push_back("ts");
	videoExtensions.push_back("m2ts");
	videoExtensions.push_back("wmv");
	videoExtensions.push_back("rm");
	videoExtensions.push_back("rmvb");
	videoExtensions.push_back("ogm");
	videoExtensions.push_back("webm");

	// ofImage uses freeImage library inder the hood. List of allowed extensions here:
	// https://freeimage.sourceforge.io/users.html
	imageExtensions.push_back("png");
	imageExtensions.push_back("jpg");
	imageExtensions.push_back("jpeg");
	imageExtensions.push_back("jp2");
	imageExtensions.push_back("bmp");
	imageExtensions.push_back("tif");
	imageExtensions.push_back("tiff");
	imageExtensions.push_back("tga");
	imageExtensions.push_back("pcx");
	imageExtensions.push_back("ico");

	setupDatGui();

	
	int windowWidth = ofGetScreenWidth();
	int windowHeight = ofGetScreenHeight() - 60;
	maxWidth = windowWidth - guiWidth * 2;
	maxHeight = windowHeight;
	ofSetWindowShape(windowWidth, windowHeight);
	ofSetWindowPosition(0, 30);

	drawArrows();
}

//--------------------------------------------------------------
void ofApp::update() {
	ofSetWindowTitle("Pixel Sortium - v" + ofToString(versionNumber) + " | FPS: " + ofToString(floor(ofGetFrameRate())));
	threshold = thresholdSlider->getValue();
	upperThreshold = upperThresholdSlider->getValue();
	angle = angleSlider->getValue();
	threadCount = threadCountSlider->getValue();
	maskOpacity = maskOpacitySlider->getValue() * 255;
	// Will add this back in later
	//maskThreshold = maskThresholdSlider->getValue() * 255;
	brushSize = brushSizeSlider->getValue();


	// This should be delegated to a watcher thread at some point
	if (directoryRefreshCounter >= 100) {
		if (imageDirectory.listDir() != imageDirCount) {
			populateImageDir(imageDirectory, imageScrollView);
			imageDirCount = imageDirectory.listDir();
		}
		if (maskDirectory.listDir() != maskDirCount) {
			populateImageDir(maskDirectory, maskImagesScrollView);
			maskDirCount = maskDirectory.listDir();
		}
		directoryRefreshCounter = 0;
	}
	directoryRefreshCounter++;
	if (started) {
		infoPanel->setActiveStatus("Sorting");
		vector<std::thread> threadList;
		for (int i = 0; i < threadCount; i++) {
			threadList.push_back(std::thread(pixelSortRow, sortingIndex, image.getWidth(), image.getHeight(), std::ref(imagePixels), std::ref(maskPixels), currentlySelectedSortParameter, threshold, upperThreshold, useMask, maskThreshold, currentImageAngle, xPadding, yPadding));
			sortingIndex += 1;

			if (sortingIndex >= image.getHeight() - 1) {
				sortingIndex = 0;
				sortComplete = true;
				
				break;
			}
		}
		for (int i = 0; i < threadList.size(); i++) {
			threadList[i].join();
		}
		image.setFromPixels(imagePixels);
		if (sortComplete) {
			infoPanel->setProgress(1);
			timeEnd = std::chrono::high_resolution_clock::now();

			long timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count();
			std::cout << "Thread Count of " << threadCount << " took " << timeTaken << " milliseconds." << std::endl;
			infoPanel->sortTimeTaken(timeTaken);
		}
		else {
			infoPanel->setProgress((float)sortingIndex / (float)image.getHeight());
		}

		if (sortComplete) {
			sortComplete = false;
			if (currentMode == Mode::Image) {
				started = false;
				sortButton->setLabel(SORTBUTTONTITLE);
				image.setFromPixels(imagePixels);
				//currentImageAngle = 0;
				infoPanel->setActiveStatus(IDLE);
			}
			else if (currentMode == Mode::Video) {
				saveFrameToVideo();
				if (videoPlayer.getCurrentFrame() >= videoPlayer.getTotalNumFrames() - 1) {
					videoPlayer.close();
					videoWriter.release();
					started = false;
					sortButton->setLabel(SORTBUTTONTITLE);
					std::cout << "Completed sorting video" << std::endl;
					infoPanel->setActiveStatus(IDLE);
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
						infoPanel->setActiveStatus("Rotating Image");
						rotateImage(angle, false);
						infoPanel->setActiveStatus("Sorting");
						paddingAddedToImage = true;
						currentImageAngle = angle;
					}
					std::cout << "Starting frame " << videoPlayer.getCurrentFrame() << " out of " << videoPlayer.getTotalNumFrames() << std::endl;
					timeStart = std::chrono::high_resolution_clock::now();
					infoPanel->setFrameCounter(videoPlayer.getCurrentFrame(), videoPlayer.getTotalNumFrames());
				}
			}
		}

		
	}
	imageScrollView->setPosition(ofGetWidth() - guiWidth * 2, datImagePanel->getHeight() + 10);
	datImagePanel->setPosition(ofGetWidth() - guiWidth * 2, 10);
	datMaskPanel->setPosition(ofGetWidth() - guiWidth, 10);
	maskImagesScrollView->setPosition(ofGetWidth() - guiWidth, datMaskPanel->getHeight() + 10);
	imageScrollView->update();
	maskImagesScrollView->update();
	datImagePanel->update();
	datMaskPanel->update();
}

// Code adapted to work with OF pulled from here: https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c/33564950#33564950
void ofApp::rotateImage(int angle, bool paddingAddedToImage) {
	if (angle == 0) {
		return;
	}
	std::chrono::steady_clock::time_point rotateStart = std::chrono::high_resolution_clock::now();
	int size = image.getWidth() * image.getHeight();
	int bpp = imagePixels.getBytesPerPixel();
	cv::Mat src;
	src = cv::Mat_<cv::Vec4b>(image.getHeight(), image.getWidth());

	// Multithread on transferring pixels from OF data structure to OpenCV data structure
	// This is a (necessary) waste of time regardless. Could potentially rewrite all of the sorting and image logic to just use OpenCV
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
	// WarpAffine is a very useful function but will cause the resulting image to be slightly blurry if not rotated at right angles
	cv::warpAffine(src, dst, rot, boxSize, cv::INTER_CUBIC);


	image.resize(dst.cols, dst.rows);
	size = image.getWidth() * image.getHeight();
	imagePixels.allocate(dst.cols, dst.rows, OF_IMAGE_COLOR_ALPHA);
	vector<std::thread> threadList2;
	// Multithread on transferring pixels back from OpenCV format to OF format...
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
	infoPanel->setActiveStatus("Saving Frame To Video");
	if (currentImageAngle != 0) {
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
		// Since the image may be rotated and have a black buffer area we rotate it and draw it to an FBO
		// which is the exact size of the original image. This allows us to essentially extract only the pixels
		// of the bordered, rotated image that we are interested in, and then can display it anywhere and any size we want
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
		imageFbo.draw(imageAnchorX, imageAnchorY, wid, hei);

		if (arrowDrawCounter > 0) {
			ofPushMatrix();
			ofTranslate(wid / 2 + imageAnchorX, hei / 2 + imageAnchorY);
			ofRotate(angle);
			int alpha = (int)min(255.0f, (float)arrowDrawCounter / (float)arrowDrawCounterStartFade * 255);
			ofSetColor(200, 200, 200, alpha);
			int arrowsWidth = arrowsFbo.getWidth() * 1.5;
			int arrowsHeight = arrowsFbo.getHeight() * 1.5;
			arrowsFbo.draw(-arrowsWidth / 2, -arrowsHeight / 2, arrowsWidth, arrowsHeight);
			ofPopMatrix();
			arrowDrawCounter--;
		}
	}
	if (mask.isAllocated()) {
		ofPushMatrix();
		ofSetColor(255, 255, 255, maskOpacity);
		mask.draw(imageAnchorX, imageAnchorY, 0, mask.getWidth() * currentRatio, mask.getHeight() * currentRatio);
		ofPopMatrix();
	}
	if (currentMouseMode == MouseMode::MaskDraw && withinUnrotatedImageBounds(mouseX, mouseY)) {
		ofNoFill();
		ofSetColor(255, 0, 0, 255);
		switch (currentBrushMode) {
		case (BrushMode::Circle):
			ofCircle(glm::vec3(mouseX, mouseY, 0), brushSize);
			break;
		case (BrushMode::Square):
			ofRect(mouseX - brushSize, mouseY - brushSize, brushSize * 2, brushSize * 2);
			break;
		case (BrushMode::ClickAndDrag):
			if (mouseDown) {
				int minX = min(clickedX, mouseX);
				int minY = min(clickedY, mouseY);
				ofRect(minX, minY, abs(clickedX - mouseX), abs(clickedY - mouseY));
			}
			break;
		}
	}
	int scaledWidth = unrotatedWidth * currentRatio;
	int scaledHeight = unrotatedHeight * currentRatio;
	ofFill();
	ofSetColor(50, 50, 50);
	ofRect(0, 0, ofGetWidth(), imageAnchorY);
	ofRect(0, 0, imageAnchorX, ofGetHeight());
	ofRect(scaledWidth + imageAnchorX, 0, ofGetWidth() - scaledWidth + imageAnchorX, ofGetHeight());
	ofRect(0, scaledHeight + imageAnchorY, maxWidth, ofGetHeight() - scaledHeight + imageAnchorY);
	ofNoFill();
	ofSetColor(70, 70, 70);
	ofRect(0, 0, ofGetWidth(), imageAnchorY);
	ofRect(0, 0, imageAnchorX, ofGetHeight());
	ofRect(scaledWidth + imageAnchorX, 0, ofGetWidth() - scaledWidth + imageAnchorX, ofGetHeight());
	ofRect(0, scaledHeight + imageAnchorY, maxWidth, ofGetHeight() - scaledHeight + imageAnchorY);

	imageScrollView->draw();
	maskImagesScrollView->draw();

	infoPanel->drawItems();
}

void ofApp::loadMask(std::string fileName) {
	ofFilePath filePath;
	std::string extension = filePath.getFileExt(fileName);
	if (std::find(imageExtensions.begin(), imageExtensions.end(), extension) != imageExtensions.end()) {
		infoPanel->setActiveStatus("Loading Mask");
		mask.clear();
		mask.load("images/masks/" + fileName);
		mask.setImageType(OF_IMAGE_COLOR_ALPHA);
		currentMaskFilename = fileName;

		maskPixels = mask.getPixels();

		for (int y = 0; y < mask.getHeight(); y++) {
			for (int x = 0; x < mask.getWidth(); x++) {
				ofColor pixel = maskPixels.getColor(x, y);
				maskPixels.setColor(x, y, ofColor(pixel.r, pixel.g, pixel.b, pixel.getBrightness()));
			}
		}
		mask.setFromPixels(maskPixels);
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
		useMask = true;
		infoPanel->setUsingMask(useMask);
	}
	infoPanel->setActiveStatus(IDLE);
}

void ofApp::loadImage(std::string fileName) {
	videoPlayer.close();
	image.clear();
	ofFilePath filePath;
	std::string extension = filePath.getFileExt(fileName);
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (std::find(imageExtensions.begin(), imageExtensions.end(), extension) != imageExtensions.end()) {
		infoPanel->setActiveStatus("Loading Image");
		image.load("images/" + fileName);
		image.setImageType(OF_IMAGE_COLOR_ALPHA);
		currentMode = Mode::Image;
		unrotatedWidth = image.getWidth();
		unrotatedHeight = image.getHeight();
		imagePixels = image.getPixels();
		paddingAddedToImage = false;
		currentImageAngle = 0;
		imageAnchorX = 0;
		imageAnchorY = 0;
		infoPanel->setFrameCounter(0, 0);
	}
	else if (std::find(videoExtensions.begin(), videoExtensions.end(), extension) != videoExtensions.end()) {
		infoPanel->setActiveStatus("Loading Video");
		if (videoPlayer.isLoaded()) {
			videoPlayer.close();
		}
		if (videoWriter.isOpened()) {
			videoWriter.release();
		}
		videoPlayer.load("images/" + fileName);
		if (!videoPlayer.isLoaded()) {
			// Display error message to user
			infoPanel->setActiveStatus(IDLE);
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
		imageAnchorX = 0;
		imageAnchorY = 0;

		
		currentMode = Mode::Video;
		infoPanel->setFrameCounter(0, videoPlayer.getTotalNumFrames());
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

	calculateCurrentRatio(unrotatedWidth, unrotatedHeight);
	calculateImageAnchorPoints(unrotatedWidth, unrotatedHeight, maxWidth, maxHeight, currentRatio);

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
	//ofSetWindowShape(unrotatedWidth * currentRatio + guiWidth * 2, unrotatedHeight * currentRatio);
	resetGuiPosition();
	sortingIndex = 0;
	started = false;
	sortButton->setLabel(SORTBUTTONTITLE);
	infoPanel->setActiveStatus(IDLE);
}

void ofApp::start(ofxDatGuiButtonEvent e) {
	if (currentMode == Mode::None) {
		return;
	}
	started = !started;
	if (started) {
		infoPanel->setActiveStatus("Sorting");
		sortButton->setLabel("Stop");
		timeStart = std::chrono::high_resolution_clock::now();
		if (angle != currentImageAngle) {
			rotateImage(angle - currentImageAngle, paddingAddedToImage);
			paddingAddedToImage = true;
			currentImageAngle = angle;
		}
		if (currentMode == Mode::Video) {
			infoPanel->setFrameCounter(videoPlayer.getCurrentFrame(), videoPlayer.getTotalNumFrames());
			float fps = videoPlayer.getTotalNumFrames() / videoPlayer.getDuration();
			videoWriter = cv::VideoWriter("data/images/" + getTimeStampedFileName(currentFileName, ".mp4", ""), cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(unrotatedWidth, unrotatedHeight), true);
		}
	}
	else {
		if (currentMode == Mode::Video) {
			infoPanel->setFrameCounter(0, 0);
		}
		infoPanel->setActiveStatus(IDLE);
		sortButton->setLabel(SORTBUTTONTITLE);
		videoWriter.release();
		videoPlayer.close();
		image.clear();
		// Display to the user that the video was saved
	}
}

void ofApp::resetGuiPosition() {
	datImagePanel->setPosition(ofGetWidth() - guiWidth * 2, 10);
	imageScrollView->setPosition(ofGetWidth() - guiWidth * 2, datImagePanel->getHeight() + 10);
	datMaskPanel->setPosition(ofGetWidth() - guiWidth, 10);
	maskImagesScrollView->setPosition(ofGetWidth() - guiWidth, datMaskPanel->getHeight() + 10);
}

void ofApp::setupDatGui() {
	// Image Panel
	datImagePanel = new ofxDatGui(ofGetWidth() - guiWidth * 2, 10);
	datImagePanel->setWidth(guiWidth);
	datImagePanel->addHeader("Image Sorting Controls");
	sortButton = datImagePanel->addButton(SORTBUTTONTITLE);
	sortButton->onButtonEvent(this, &ofApp::start);
	ofxDatGuiButton* saveButton = datImagePanel->addButton(SAVEIMAGEBUTTONTITLE);
	saveButton->onButtonEvent(this, &ofApp::saveCurrentImage);
	ofxDatGuiButton* revertButton = datImagePanel->addButton("Revert Changes");
	revertButton->onButtonEvent(this, &ofApp::revertChanges);
	ofxDatGuiDropdown* sortingParameterDropdown = datImagePanel->addDropdown("Sorting Parameter", sortingParameterOptions);
	sortingParameterDropdown->onDropdownEvent(this, &ofApp::selectSortingParameter);
	sortingParameterDropdown->select(0);
	thresholdSlider = datImagePanel->addSlider(LOWERTHRESHOLDTITLE, 0.0f, 1.0f, 0.25f);
	upperThresholdSlider = datImagePanel->addSlider(UPPERTHRESHOLDTITLE, 0.0f, 1.0f, 0.80f);
	angleSlider = datImagePanel->addSlider(ANGLESLIDERTITLE, 0, 359, 0);
	angleSlider->setPrecision(0);
	angleSlider->onSliderEvent(this, &ofApp::angleSliderChanged);
	threadCountSlider = datImagePanel->addSlider(THREADCOUNTSLIDERTITLE, 1, 30, threadCount);
	threadCountSlider->setPrecision(0);
	threadCountSlider->setValue(threadCount);
	datImagePanel->addLabel("Load Image");
	imageScrollView = new ofxDatGuiScrollView("Image/Video Files", (ofGetHeight() - datImagePanel->getHeight()) / 26 - 1);
	imageScrollView->setWidth(guiWidth);
	imageScrollView->setPosition(ofGetWidth() - guiWidth * 2, datImagePanel->getHeight() + 10);
	imageScrollView->onScrollViewEvent(this, &ofApp::clickOnImageButton);
	imageDirectory.open("images");
	for (int i = 0; i < imageExtensions.size(); i++) {
		imageDirectory.allowExt(imageExtensions[i]);
	}
	for (int i = 0; i < videoExtensions.size(); i++) {
		imageDirectory.allowExt(videoExtensions[i]);
	}
	populateImageDir(imageDirectory, imageScrollView);
	imageDirCount = imageDirectory.listDir();

	// Mask Panel
	datMaskPanel = new ofxDatGui(ofGetWidth() - guiWidth, 10);
	datMaskPanel->setWidth(guiWidth);
	datMaskPanel->addHeader("Mask Controls");
	ofxDatGuiButton* clearMaskButton = datMaskPanel->addButton("Clear Mask");
	clearMaskButton->onButtonEvent(this, &ofApp::clearMask);
	ofxDatGuiButton* saveMaskButton = datMaskPanel->addButton("Save Mask");
	saveMaskButton->onButtonEvent(this, &ofApp::saveCurentMask);
	invertMaskButton = datMaskPanel->addButton("Invert Mask");
	invertMaskButton->onButtonEvent(this, &ofApp::invertMask);
	maskOpacitySlider = datMaskPanel->addSlider(MASKOPACITYTITLE, 0.0, 1.0, 0.4);
	//maskThresholdSlider = datMaskPanel->addSlider("Mask Threshold", 0.0, 1.0, 1.0);
	maskBrushToggle = datMaskPanel->addButton(DRAWMASKTOOLTITLE);
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
	for (int i = 0; i < imageExtensions.size(); i++) {
		maskDirectory.allowExt(imageExtensions[i]);
	}
	populateImageDir(maskDirectory, maskImagesScrollView);
	maskDirCount = maskDirectory.listDir();

	
	infoPanel = new InfoPanel();
	infoPanel->setup();
}

void ofApp::populateImageDir(ofDirectory dir, ofxDatGuiScrollView* scrollView) {
	scrollView->clear();
	size_t dirSize = dir.listDir();
	for (int i = 0; i < dirSize; i++) {
		scrollView->add(dir.getName(i));
		scrollView->getItemAtIndex(i)->setLabelUpperCase(false);
	}
}

void ofApp::saveCurrentImage(ofxDatGuiButtonEvent e) {
	if (image.isAllocated() && !started) {
		infoPanel->setActiveStatus("Saving Image");
		ofPixels copy;
		copy.allocate(imagePixels.getWidth(), imagePixels.getHeight(), imagePixels.getImageType());
		imagePixels.pasteInto(copy, 0, 0);
		rotateImage(-currentImageAngle, paddingAddedToImage);
		int originalImageX = image.getWidth() / 2 - unrotatedWidth / 2;
		int originalImageY = image.getHeight() / 2 - unrotatedHeight / 2;
		imagePixels.crop(originalImageX, originalImageY, unrotatedWidth, unrotatedHeight);
		image.resize(imagePixels.getWidth(), imagePixels.getHeight());
		image.setFromPixels(imagePixels);
		std::string fullName = getTimeStampedFileName(currentFileName, "", "");
		image.save("images/" + fullName);
		currentFileName = fullName;
		imagePixels = copy;
		image.setFromPixels(imagePixels);
	}
	infoPanel->setActiveStatus(IDLE);
}

void ofApp::saveCurentMask(ofxDatGuiButtonEvent e) {
	if (mask.isAllocated()) {
		std::string saveName = currentMaskFilename;
		if (currentMaskFilename == "") {
			ofFilePath filePath;
			saveName = filePath.getBaseName(currentFileName);
		}
		std::string fullName = getTimeStampedFileName(saveName, ".png", "Mask");
		mask.save("images/masks/" + fullName);
	}
}

std::string ofApp::getTimeStampedFileName(std::string filename, std::string suppliedExtension, std::string suffix) {
	ofFilePath filePath;
	std::string baseName = filePath.getBaseName(filename);
	std::string extension = "." + filePath.getFileExt(filename);
	bool alreadyHasDate = true;
	int fileNameLength = static_cast<int>(baseName.length());
	for (int i = 0; i < 14; i++) {
		int index = fileNameLength - i - 1;
		if (index < 0) {
			alreadyHasDate = false;
			break;
		}
		unsigned char c = baseName.at(index);
		if (!isdigit(c)) {
			alreadyHasDate = false;
			break;
		}
	}
	alreadyHasDate = alreadyHasDate && baseName.at(max(fileNameLength - 15, 0)) == '-';

	if (alreadyHasDate) {
		baseName = baseName.substr(0, baseName.length() - 15);
	}
	baseName += suffix;
	std::string extensionToUse = extension;
	if (suppliedExtension != "") {
		extensionToUse = suppliedExtension;
	}
	return baseName + "-" + datetime() + extensionToUse;
}

std::string ofApp::datetime()
{
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%Y%m%d%H%M%S", timeinfo);
	return std::string(buffer);
}

void ofApp::clickOnMaskImageButton(ofxDatGuiScrollViewEvent e) {
	loadMask(e.target->getName());
}

void ofApp::clickOnImageButton(ofxDatGuiScrollViewEvent e) {
	loadImage(e.target->getName());
}

void ofApp::maskToolToggleClicked(ofxDatGuiButtonEvent e) {
	if (currentMouseMode != MouseMode::MaskDraw) {
		maskBrushToggle->setBackgroundColor(ofColor(0, 166, 0));
		currentMouseMode = MouseMode::MaskDraw;
	}
	else {
		maskBrushToggle->setBackgroundColor(ofColor::fromHex(0x1A1A1A));
		currentMouseMode = MouseMode::Default;
	}
}

void ofApp::angleSliderChanged(ofxDatGuiSliderEvent e) {
	arrowDrawCounter = arrowDrawCounterReset;
}

void ofApp::brushTypeSelected(ofxDatGuiDropdownEvent e) {
	currentBrushMode = (BrushMode)e.child;
}

void ofApp::selectSortingParameter(ofxDatGuiDropdownEvent e) {
	currentlySelectedSortParameter = (SortParameter)e.child;
}

void ofApp::revertChanges(ofxDatGuiButtonEvent e) {
	if (currentMode == Mode::Image) {
		loadImage(currentFileName);
	}
}

void ofApp::clearMask(ofxDatGuiButtonEvent e) {
	mask.clear();
	mask.allocate(unrotatedWidth, unrotatedHeight, OF_IMAGE_COLOR_ALPHA);
	mask.setColor(ofColor(0, 0, 0, 0));
	mask.update();
	maskPixels = mask.getPixels();
	useMask = false;
	currentMaskFilename = "";
	infoPanel->setUsingMask(useMask);
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
					maskPixels.setColor(modX, modY, ofColor(255, 255, 255, value));
					useMask = true;
				}
			}
		}
	}
	infoPanel->setUsingMask(useMask);
	mask.setFromPixels(maskPixels);
	maskPixels = mask.getPixels();
}

bool ofApp::withinMaskBounds(int x, int y) {
	return x >= imageAnchorX && x < mask.getWidth() + imageAnchorX && y >= imageAnchorY && y < mask.getHeight() + imageAnchorY;
}

bool ofApp::withinUnrotatedImageBounds(int x, int y) {
	return x >= imageAnchorX && x < unrotatedWidth * currentRatio + imageAnchorX && y >= imageAnchorY && y < unrotatedHeight * currentRatio + imageAnchorY;
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

void ofApp::drawArrows() {
	int lineSpacing = 40;
	int arrowSpacing = 80;
	int largerSide = max(maxWidth, maxHeight);
	int linesToDraw = largerSide * 4 / lineSpacing;
	int arrowWidth = 7;
	arrowsFbo.clear();
	arrowsFbo.allocate(largerSide, largerSide);
	arrowsFbo.begin();
	ofClear(0);
	ofSetColor(255);
	ofPushMatrix();
	ofRotate(90);
	for (int i = 0; i < linesToDraw; i++) {
		ofSetLineWidth(1.0f);
		int x = -largerSide * 2 + lineSpacing * i;
		ofDrawLine(x, -largerSide * 2, x, largerSide * 2);
		int arrowsToDraw = largerSide * 4 / arrowSpacing;
		for (int c = 0; c < arrowsToDraw; c++) {
			int y = -largerSide * 2 + arrowSpacing * c;
			ofSetLineWidth(5.0f);
			ofDrawTriangle(x, y, x + arrowWidth, y + arrowWidth, x - arrowWidth, y + arrowWidth);
		}
	}
	ofPopMatrix();
	arrowsFbo.end();
}

void ofApp::calculateCurrentRatio(int width, int height) {
	if (width > maxWidth || height > maxHeight) {
		float xRatio = (float)maxWidth / (float)width;
		float yRatio = (float)maxHeight / (float)height;
		currentRatio = xRatio < yRatio ? xRatio : yRatio;
	}
	else {
		currentRatio = 1.0f;
	}
}

void ofApp::calculateImageAnchorPoints(int unrotatedWidth, int unrotatedHeight, int maxWidth, int maxHeight, float ratio) {
	int scaledWidth = unrotatedWidth * ratio;
	int scaledHeight = unrotatedHeight * ratio;
	imageAnchorX = max(0, (maxWidth - scaledWidth) / 2);
	imageAnchorY = max(0, (maxHeight - scaledHeight) / 2 + guiHeight);
}

void ofApp::invertMask(ofxDatGuiButtonEvent e) {
	if (mask.isAllocated()) {
		for (int y = 0; y < mask.getHeight(); y++) {
			for (int x = 0; x < mask.getWidth(); x++) {
				ofColor pixel = maskPixels.getColor(x, y);
				maskPixels.setColor(x, y, ofColor(255, 255, 255, 255 - pixel.a));
			}
		}
	}
	mask.setFromPixels(maskPixels);
	maskPixels = mask.getPixels();
	useMask = true;
	infoPanel->setUsingMask(useMask);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	if (x > ofGetWidth() - guiWidth * 2) {
		if (x < ofGetWidth() - guiWidth) {
			datImagePanel->focus();
		}
		else {
			datMaskPanel->focus();
		}
		
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if (currentMouseMode == MouseMode::MaskDraw && withinUnrotatedImageBounds(x, y)) {
		if (!(currentBrushMode == BrushMode::ClickAndDrag)) {
			if (dragCounter % 2 == 0) {
				std::cout << "Dragged: " << x << ", " << y << std::endl;
				applyBrushStroke(x - imageAnchorX, y - imageAnchorY, brushSize, currentBrushMode, 255 * (abs(button - 2) / 2));
			} 
			dragCounter++;
		}
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (currentMouseMode == MouseMode::MaskDraw && withinUnrotatedImageBounds(x, y)) {
		mouseDown = true;
		clickedX = x;
		clickedY = y;
		buttonDown = button;
		if (currentBrushMode == BrushMode::ClickAndDrag) {
		}
		else {
			applyBrushStroke(x - imageAnchorX, y - imageAnchorY, brushSize, currentBrushMode, 255 * (abs(button - 2) / 2));
		}
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
	datMaskPanel->getSlider(BRUSHSIZESLIDERTITLE)->setValue(brushSize);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	
	// Left click button value is 0, right click button value is 2;
	// The below calculation comes to 255 for left click and 0 for right click
	int value = 255 * abs(button - 2) / 2;
	if (currentMouseMode == MouseMode::MaskDraw && withinUnrotatedImageBounds(x, y)) {
		if (currentBrushMode == BrushMode::ClickAndDrag) {
			int minX = min(clickedX, x) / currentRatio;
			int minY = min(clickedY, y) / currentRatio;
			int maxX = max(clickedX, x) / currentRatio;
			int maxY = max(clickedY, y) / currentRatio;
			for (int row = minY; row < maxY; row++) {
				for (int col = minX; col < maxX; col++) {
					if (withinMaskBounds(col, row)) {
						maskPixels.setColor(col - imageAnchorX, row - imageAnchorY, ofColor(255, 255, 255, value));
					}
				}
			}
			mask.setFromPixels(maskPixels);
			maskPixels = mask.getPixels();
			useMask = true;
			infoPanel->setUsingMask(useMask);
		}
	}
	mouseDown = false;
	dragCounter = 0;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	imageScrollView->setNumVisible((h - datImagePanel->getHeight()) / 26 - 1);
	maskImagesScrollView->setNumVisible((h - datMaskPanel->getHeight()) / 26 - 1);
	maxWidth = w - guiWidth * 2;
	maxHeight = h - guiHeight;
	calculateCurrentRatio(unrotatedWidth, unrotatedHeight);
	calculateImageAnchorPoints(unrotatedWidth, unrotatedHeight, maxWidth, maxHeight, currentRatio);
	infoPanel->setItemPositions(0, 0, ofGetWidth() - guiWidth * 2);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}



