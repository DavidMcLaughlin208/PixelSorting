#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include <vector>
#include <string>
#include <chrono>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>

class ofApp : public ofBaseApp{

	public:
		enum class Mode {
			None,
			Image,
			Video
		};


		enum class SortParameter {
			Brightness,
			Hue,
			Saturation
		};

		void rotateImage(int angle, bool paddingAddedToImage);
		void setup();
		void update();
		void convertVecToCharPixels(vector<unsigned char> &charVec, glm::vec3* vecPointer, int bytesPerPixel, int pixelsBufferSize);
		void draw();
		
		void saveFrameToVideo();
		void start();
		void selectParameterRadioButton(const void* sender);
		bool clickedOnLabel(const void* sender);

		void loadImage(std::string fileName);

		void resetGuiPosition();

		void setupGui();

		void saveCurrentImage();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		static std::string BRIGHTNESS;
		static std::string LIGHTNESS;
		static std::string HUE;
		static std::string SATURATION;
		
		ofDirectory directory;
		ofImage image;
		int unrotatedWidth;
		int unrotatedHeight;

		//ofxCvColorImage cvImage;
		ofPixels pixels;

		ofxPanel gui;
		int guiWidth = 200;
		ofxFloatSlider thresholdSlider;
		ofxFloatSlider upperThresholdSlider;
		ofxFloatSlider ratioSlider;
		ofxFloatSlider ratioSliderY;
		ofxIntSlider angleSlider;
		vector<ofxButton*> buttons;
		ofxButton sortButton;
		ofxButton saveButton;
		char pixelSwapBuffer[4];

		cv::RotatedRect currentRotatedRect;

		int sortingIndex = 0;
		bool started = false;
		std::string currentFileName;

		float threshold = 0.25f;
		float upperThreshold = 0.8f;
		int angle = 0;
		int currentImageAngle = 0;
		bool sortComplete = false;
		bool paddingAddedToImage = false;

		int threadCount = 10;
		ofxIntSlider threadCountSlider;

		cv::VideoWriter videoWriter;
		bool createVideo = true;
		ofVideoPlayer videoPlayer;

		std::chrono::steady_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
		std::chrono::steady_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();

		std::set<std::string> videoExtensions;
		std::set<std::string> imageExtensions;


		// Threshold parameter radio buttons
		ofxButton brightnessRadio;
		ofxButton hueRadio;
		ofxButton saturationRadio;
		vector<ofxButton> thresholdValueRadioButtons;
		SortParameter currentlySelectedThresholdVariable = SortParameter::Brightness;
		ofxLabel selectedThresholdVariable;

		Mode currentMode = Mode::None;
		std::map<std::string, SortParameter> sortParameterTable;


};