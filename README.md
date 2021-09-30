# Pixel Sorting

### What is pixel sorting?

In short, pixel sorting is taking an interval of pixels in a row or column from an image and sorting them based on some criteria.

The intervals can be determined arbitrarily, such as seleccting all consecutive pixels under a certain brightness threshold, and then sorting them by some property such as brightness values.

For more details and a visual explanation check this blog post by satyarth [here](http://satyarth.me/articles/pixel-sorting/).

### Why make this program?

satyarth has already created a comprehensive [command line tool](https://github.com/satyarth/pixelsort) written in Python to perform pixel sorting on images. 
I wanted to be able to easily apply pixel sorting to videos files but that would require a lot of manual work using existing tools. 

Additionally, I wanted to implement multithreading on the pixel sort algorithm to speed it up which will be especially useful for sorting long videos.
The single threaded python application can sort a 1920x1200 image with thresholds set from 0-1 (worst case scenario where every pixel is sorted) in about 9 seconds.
This c++ multithreaded application can perform the same sort on the same image in 2.5 seconds while showing the image as its being sorted.

I also wanted to created an application with a gui to give more immediate visual feedback to the user and add other tools such as mask drawing in app.

### Dependencies

This application uses [OpenFrameworks](https://openframeworks.cc/) and c++ for the pixel sorting, image loading, and video loading/playback.
[OpenCV](https://opencv.org/) is used to take the modified pixels from each frame and collect them into an .mp4 file

### Usage

This application is still in development. I plan to publish some iterations in the Releases section when the code is in a more user friendly state.

### Todo List:

* Add masking functionality
  * Add GUI panel for mask info
  * Use mask to determine eligible pixels for sorting
  * Use mask as intervals
  * Add ability to draw custom mask in application
* Improve UI clarity and usability
 * Add diagnostics and metrics to be shown in app (time taken, current frame for videos)
 * Add ability to apply curves or easing functions to parameter values to change them over the course of a video sorting
 * Clean up GUI panels and logically separate buttons, parameters, etc.
 * Allow for reload of image/video load buttons when contents of folder change
* Add undo functionality

### Examples

![](media/MultiSort.gif)
![](media/SortingOptions.gif)