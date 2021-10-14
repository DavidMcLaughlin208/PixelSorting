#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "ofxDatGui.h"
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

		enum class MouseMode {
			Default,
			MaskDraw
		};

		enum class BrushMode {
			Circle,
			Square
		};


		void setup();
		void update();
		void draw();
		
		void rotateImage(int angle, bool paddingAddedToImage);
		void convertVecToCharPixels(vector<unsigned char>& charVec, glm::vec3* vecPointer, int bytesPerPixel, int pixelsBufferSize);
		
		void saveFrameToVideo();
		void start();
		void selectParameterRadioButton(const void* sender);
		bool clickedOnImageButton(const void* sender);
		bool maskToolToggleClicked();
		bool clickOnMaskImageButton(const void* sender);
		void applyBrushStroke(int x, int y, int size, ofApp::BrushMode mode, int value);
		bool withinMaskBounds(int x, int y);
		bool withinUnrotatedImageBounds(int x, int y);
		bool cycleBrushMode();
		virtual void mouseScrolled(int x, int y, float scrollX, float scrollY);

		void loadMask(std::string fileName);
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
		int xPadding;
		int yPadding;

		ofImage mask;
		bool useMask = false;
		int maskOpacity = 110;

		ofPixels imagePixels;
		ofPixels maskPixels;

		ofxPanel gui;
		ofxPanel maskPanel;
		int guiWidth = 200;
		ofxFloatSlider thresholdSlider;
		ofxFloatSlider upperThresholdSlider;
		ofxIntSlider angleSlider;
		ofxToggle maskToggle;
		ofxFloatSlider maskOpacitySlider;
		
		ofxButton maskToolToggle;
		vector<ofxButton*> buttons;
		vector<ofxButton*> maskFileButtons;
		ofxButton sortButton;
		ofxButton saveButton;
		char pixelSwapBuffer[4];

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
		int pixelTransferThreadCount = 14;

		cv::VideoWriter videoWriter;
		bool createVideo = true;
		ofVideoPlayer videoPlayer;

		std::chrono::steady_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
		std::chrono::steady_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();

		std::set<std::string> videoExtensions;
		std::set<std::string> imageExtensions;

		// Brush variables
		int brushSize = 5;
		BrushMode currentBrushMode = BrushMode::Circle;
		ofxIntSlider maskBrushSizeSlider;
		ofxButton brushModeCycler;


		// Threshold parameter radio buttons
		ofxButton brightnessRadio;
		ofxButton hueRadio;
		ofxButton saturationRadio;
		vector<ofxButton> thresholdValueRadioButtons;
		SortParameter currentlySelectedThresholdVariable = SortParameter::Brightness;
		ofxLabel selectedThresholdVariable;

		Mode currentMode = Mode::None;
		MouseMode currentMouseMode = MouseMode::Default;
		std::map<std::string, SortParameter> sortParameterTable;


};