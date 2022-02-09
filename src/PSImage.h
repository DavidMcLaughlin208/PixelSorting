#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>

class PSImage {
public:
	void load(std::string fileName);
	void calculateCurrentRatio(int maxWidth, int maxHeight);
	void calculateImageAnchorPoints(int maxWidth, int maxHeight, int guiHeight);
	void rotateImage(int angle);
	void save(std::string timestampedFileName);
	void setFromMat(cv::Mat cvImg);
	void init(std::string imagesPath);

	cv::Mat cvImg;
	ofImage ofImg;
	ofFilePath filePath;
	ofPixels imagePixels;
	int unrotatedWidth;
	int unrotatedHeight;
	int scaledWidth;
	int scaledHeight;
	bool paddingAddedToImage;
	int currentImageAngle;
	int imageAnchorX;
	int imageAnchorY;
	int xPadding;
	int yPadding;
	float currentRatio;
	std::string imagesPath;
	std::string currentFileName;
private:

};