#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxDatGui.h"
#include "InfoPanel.h"
#include "../resource.h"
#include <vector>
#include <string>
#include <chrono>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>

class ofApp : public ofBaseApp {

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
		Square,
		ClickAndDrag
	};


	void setup();
	void update();
	void draw();

	void drawArrows();
	void createPolyLine();

	void rotateImage(int angle, bool paddingAddedToImage);
	void saveFrameToVideo();
	void start(ofxDatGuiButtonEvent e);
	void revertChanges(ofxDatGuiButtonEvent e);
	void maskToolToggleClicked(ofxDatGuiButtonEvent e);
	void clickOnMaskImageButton(ofxDatGuiScrollViewEvent e);
	void clickOnImageButton(ofxDatGuiScrollViewEvent e);
	void brushTypeSelected(ofxDatGuiDropdownEvent e);
	void selectSortingParameter(ofxDatGuiDropdownEvent e);
	void saveCurentMask(ofxDatGuiButtonEvent e);
	void clearMask(ofxDatGuiButtonEvent e);
	void invertMask(ofxDatGuiButtonEvent e);
	void angleSliderChanged(ofxDatGuiSliderEvent e);
	void applyBrushStroke(int x, int y, int size, ofApp::BrushMode mode, int value);
	bool withinMaskBounds(int x, int y);
	bool withinUnrotatedImageBounds(int x, int y);
	bool cycleBrushMode();
	virtual void mouseScrolled(int x, int y, float scrollX, float scrollY);

	void calculateCurrentRatio(int width, int height);
	void calculateImageAnchorPoints(int unrotatedWidth, int unrotatedHeight, int maxWidth, int maxHeight, float ratio);

	void loadMask(std::string fileName);
	void loadImage(std::string fileName);
	void populateImageDir(ofDirectory dir, ofxDatGuiScrollView* scrollView);

	void resetGuiPosition();
	void setupDatGui();

	void saveCurrentImage(ofxDatGuiButtonEvent e);
	std::string getTimeStampedFileName(std::string filename, std::string suppliedExtension, std::string suffix);
	std::string datetime();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
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
	static std::string MASKOPACITYTITLE;
	static std::string DRAWMASKTOOLTITLE;
	static std::string BRUSHSIZESLIDERTITLE;
	static std::string CIRCLE;
	static std::string SQUARE;
	static std::string CLICKANDDRAG;
	static std::string SORTBUTTONTITLE;
	static std::string SAVEIMAGEBUTTONTITLE;
	static std::string LOWERTHRESHOLDTITLE;
	static std::string UPPERTHRESHOLDTITLE;
	static std::string ANGLESLIDERTITLE;
	static std::string THREADCOUNTSLIDERTITLE;
	static std::string IDLE;

	ofDirectory imageDirectory;
	ofDirectory maskDirectory;
	int directoryRefreshCounter = 0;
	size_t imageDirCount = 0;
	size_t maskDirCount = 0;
	ofImage image;
	int unrotatedWidth;
	int unrotatedHeight;
	int xPadding;
	int yPadding;
	int imageAnchorX = 0;
	int imageAnchorY = 0;

	ofImage mask;
	bool useMask = false;
	std::string currentMaskFilename = "";
	int maskOpacity = 110;
	int maskThreshold = 255;

	ofPixels imagePixels;
	ofPixels maskPixels;

	ofFbo imageFbo;
	int guiWidth = 300;
	int guiHeight = 26;

	ofFbo arrowsFbo;
	int arrowDrawCounterReset = 200;
	int arrowDrawCounter = 0;
	int arrowDrawCounterStartFade = arrowDrawCounterReset / 2;

	int sortingIndex = 0;
	bool started = false;
	std::string currentFileName;

	// Sorting parameter
	float threshold = 0.25f;
	float upperThreshold = 0.8f;
	int angle = 0;
	int currentImageAngle = 0;
	bool sortComplete = false;
	bool paddingAddedToImage = false;

	int threadCount = 17;
	int pixelTransferThreadCount = 14;

	cv::VideoWriter videoWriter;
	ofVideoPlayer videoPlayer;

	std::chrono::steady_clock::time_point timeStart = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point timeEnd = std::chrono::high_resolution_clock::now();

	std::vector<std::string> videoExtensions;
	std::vector<std::string> imageExtensions;

	int maxWidth = 1280;
	int maxHeight = 800;
	float currentRatio = 1.0f;

	bool mouseDown = false;
	int buttonDown = 0;
	int clickedX;
	int clickedY;
	int dragCounter = 0;

	// Brush variables
	int brushSize = 5;
	BrushMode currentBrushMode = BrushMode::Circle;

	// Value by which to sort
	Mode currentMode = Mode::None;
	MouseMode currentMouseMode = MouseMode::Default;
	SortParameter currentlySelectedSortParameter = SortParameter::Brightness;

	//ofxDatGui
	ofxDatGui* datImagePanel;
	ofxDatGuiScrollView* imageScrollView;;
	vector<string> sortingParameterOptions;
	ofxDatGuiButton* sortButton;
	ofxDatGuiSlider* thresholdSlider;
	ofxDatGuiSlider* upperThresholdSlider;
	ofxDatGuiSlider* angleSlider;
	ofxDatGuiSlider* threadCountSlider;

	ofxDatGui* datMaskPanel;
	ofxDatGuiScrollView* maskImagesScrollView;
	vector<string> brushTypeOptions;
	ofxDatGuiSlider* maskOpacitySlider;
	ofxDatGuiSlider* brushSizeSlider;
	ofxDatGuiToggle* useMaskToggle;
	ofxDatGuiSlider* maskThresholdSlider;
	ofxDatGuiButton* invertMaskButton;
	ofxDatGuiButton* maskBrushToggle;

	InfoPanel* infoPanel;

	ofColor averageColorOfImage;

	float versionNumber = 0.1.0;
};