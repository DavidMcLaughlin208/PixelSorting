#include "PSImage.h"

void PSImage::init(std::string imagesPath) {
	this->imagesPath = imagesPath;
}

void PSImage::load(std::string fileName) {
	ofImg.load(imagesPath + fileName);
	ofImg.setImageType(OF_IMAGE_COLOR_ALPHA);
	imagePixels = ofImg.getPixels();

	unrotatedWidth = ofImg.getWidth();
	unrotatedHeight = ofImg.getHeight();
	paddingAddedToImage = false;
	currentImageAngle = 0;
	imageAnchorX = 0;
	imageAnchorY = 0;
	xPadding = 0;
	yPadding = 0;
	currentFileName = fileName;
}

void PSImage::setFromMat(cv::Mat cvImg) {
	ofImg.clear();
	ofImg.resize(cvImg.cols, cvImg.rows);
	ofImg.setFromPixels(cvImg.data, cvImg.cols, cvImg.rows, OF_IMAGE_COLOR_ALPHA);
	imagePixels = ofImg.getPixels();

	unrotatedWidth = cvImg.cols;
	unrotatedHeight = cvImg.rows;
	paddingAddedToImage = false;
	currentImageAngle = 0;
	imageAnchorX = 0;
	imageAnchorY = 0;
	xPadding = 0;
	yPadding = 0;
}

void PSImage::calculateCurrentRatio(int maxWidth, int maxHeight) {
	if (this->unrotatedWidth > maxWidth || this->unrotatedHeight > maxHeight) {
		float xRatio = (float)maxWidth / (float)this->unrotatedWidth;
		float yRatio = (float)maxHeight / (float)this->unrotatedHeight;
		this->currentRatio = xRatio < yRatio ? xRatio : yRatio;
	}
	else {
		this->currentRatio = 1.0f;
	}
	this->scaledWidth = this->unrotatedWidth * this->currentRatio;
	this->scaledHeight = this->unrotatedHeight * this->currentRatio;
}

void PSImage::calculateImageAnchorPoints(int maxWidth, int maxHeight, int guiHeight) {
	int scaledWidth = this->unrotatedWidth * this->currentRatio;
	int scaledHeight = this->unrotatedHeight * this->currentRatio;
	this->imageAnchorX = max(0, (maxWidth - scaledWidth) / 2);
	this->imageAnchorY = max(0, (maxHeight - scaledHeight) / 2 + guiHeight);
}

// Code adapted to work with OF pulled from here : https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c/33564950#33564950
void PSImage::rotateImage(int angle) {
	int angleDiff = angle - currentImageAngle;
	if (currentImageAngle == angle) {
		return;
	}
	std::chrono::steady_clock::time_point rotateStart = std::chrono::high_resolution_clock::now();
	int size = ofImg.getWidth() * ofImg.getHeight();
	int bpp = imagePixels.getBytesPerPixel();
	cv::Mat src;
	src = cv::Mat_<cv::Vec4b>(ofImg.getHeight(), ofImg.getWidth());
	src.data = imagePixels.getData();
	cvtColor(src, src, cv::COLOR_RGBA2BGRA);

	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
	cv::Mat rot = cv::getRotationMatrix2D(center, angleDiff, 1.0);
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
		paddingAddedToImage = true;
	}
	// adjust transformation matrix
	rot.at<double>(0, 2) += boxSize.width / 2.0 - src.cols / 2.0;
	rot.at<double>(1, 2) += boxSize.height / 2.0 - src.rows / 2.0;

	cv::Mat dst;
	// WarpAffine is a very useful function but will cause the resulting image to be slightly blurry if not rotated at right angles
	cv::warpAffine(src, dst, rot, boxSize, cv::INTER_CUBIC);

	cvtColor(dst, dst, cv::COLOR_BGRA2RGBA);
	ofImg.resize(dst.cols, dst.rows);
	ofImg.setFromPixels(dst.data, dst.cols, dst.rows, OF_IMAGE_COLOR_ALPHA);
	imagePixels = ofImg.getPixels();

	currentImageAngle = angle;

	std::chrono::steady_clock::time_point rotateEnd = std::chrono::high_resolution_clock::now();
	long timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(rotateEnd - rotateStart).count();
	std::cout << "Rotating image took " << timeTaken << " milliseconds for rotation diff of " << angle << " degrees." << std::endl;
}

//void PSImage::unrotateAndCrop() {
//
//}

void PSImage::save(std::string timestampedFileName) {
	if (ofImg.isAllocated()) {
		ofPixels copy;
		copy.allocate(imagePixels.getWidth(), imagePixels.getHeight(), imagePixels.getImageType());
		imagePixels.pasteInto(copy, 0, 0);
		rotateImage(-currentImageAngle);
		int originalImageX = ofImg.getWidth() / 2 - unrotatedWidth / 2;
		int originalImageY = ofImg.getHeight() / 2 - unrotatedHeight / 2;
		imagePixels.crop(originalImageX, originalImageY, unrotatedWidth, unrotatedHeight);
		ofImg.resize(imagePixels.getWidth(), imagePixels.getHeight());
		ofImg.setFromPixels(imagePixels);
		ofImg.save(imagesPath + timestampedFileName);
		currentFileName = timestampedFileName;
		imagePixels = copy;
		ofImg.setFromPixels(imagePixels);
	}
}