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



		void setup();
		void update();
		void convertVecToCharPixels(vector<unsigned char> &charVec, glm::vec3* vecPointer, int bytesPerPixel, int pixelsBufferSize);
		void draw();
		void pixelSort();
		
		void saveFrameToVideo();
		void swapPixels(ofPixels& pixels, int index1, int index2, int bytesPerPixel);
		int getActualIndex(int index, int column, int bytesPerPixel, int imageWidth, bool isHorizontal);
		float getThresholdVariableFromColor(ofColor color, std::string selectedVariable);
		void start();
		void selectParameterRadioButton(const void* sender);
		bool clickedOnLabel(const void* sender);

		void loadImage(std::string fileName);

		void resetGuiPosition();

		void setupGui();

		void setupShaders();

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
		ofTexture texture;
		ofTexture sortedTexture;
		ofImage sortedImage;
		ofPixels pixels;
		ofFbo fbo;

		ofShader pixelSortCompute;
		ofBufferObject pixelsBuffer;
		vector<glm::vec3> pixelAllocater;
		ofShader shader;

		ofxPanel gui;
		int guiWidth = 200;
		ofxFloatSlider thresholdSlider;
		ofxFloatSlider upperThresholdSlider;
		vector<ofxButton*> buttons;
		ofxButton sortButton;
		ofxButton saveButton;
		ofxToggle horizontalToggle;
		ofxToggle reverseSort;
		ofxLabel reverseSortLabel;
		ofxLabel horiztonalToggleLabel;
		char pixelSwapBuffer[4];

		int sortingIndex = 0;
		bool started = false;
		std::string currentFileName;

		float threshold = 0.25f;
		float upperThreshold = 0.8f;
		bool horizontal = false;
		bool reverse = false;
		bool useCompute = false;
		bool useThreads = true;
		bool sortComplete = false;

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