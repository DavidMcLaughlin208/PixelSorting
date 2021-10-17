# Pixel Sorting

### What is pixel sorting?

In short, pixel sorting is taking an interval of pixels in a row or column from an image and sorting them based on some criteria.

The intervals can be determined arbitrarily, such as seleccting all consecutive pixels under a certain brightness threshold, and then sorting them by some property such as brightness values.

For more details and a visual explanation check this blog post by satyarth [here](http://satyarth.me/articles/pixel-sorting/).

![](media/SortedVideo.gif)

### Why make this program?

satyarth has already created a comprehensive [command line tool](https://github.com/satyarth/pixelsort) written in Python to perform pixel sorting on images. 
I wanted to be able to easily apply pixel sorting to videos files but that would require a lot of manual work using existing tools. 

Additionally, I wanted to implement multithreading on the pixel sort algorithm to speed it up which will be especially useful for sorting long videos.
The single threaded python application can sort a 1920x1200 image with thresholds set from 0-1 (worst case scenario where every pixel is sorted) in about 9 seconds.
This c++ multithreaded application can perform the same sort on the same image in 4.2 seconds while showing the image as its being sorted.

I also wanted to created an application with a gui to give more immediate visual feedback to the user and add other tools such as mask drawing in app.

### Dependencies

This application uses [OpenFrameworks](https://openframeworks.cc/) and c++ for the pixel sorting, image loading, and video loading/playback.
[OpenCV](https://opencv.org/) is used to take the modified pixels from each frame and collect them into an .mp4 file

### Usage

This application is still in development. I plan to publish some iterations in the Releases section when the code is in a more user friendly state.
Documentation and a walkthrough video will be made as well.

### Caveats

In order for an image to be sorted at any angle, each image pixel needs to be rotated within a 2D matrix. Since contents of a 2D matrix cannot be cleanly rotated at anything other than 90, 180, or 270 degrees, the 'warpAffine' function
used by OpenCV will perform some color interpolation on the pixels to make the rotated image retain the same dimensions and all have the pixels still be adjacent to each other. After the sorting is performed, when saving an image, it must be rotated back to a 0 degree orientation.
These multiple rotations will cause the image to become slightly blurred (not very noticable though) and there may be some color artifacts at parts of the image borders. Additionally, these rotations increase the time it takes for each sort. This can signficantly increase the time required to
sort a video as each frame must be rotated, sorted, and then rotated back to be saved to the new video file.

None of the above issues apply when sorting an image at 0 degree angle (default, horiztonal)

---

When sorting images it is recommended to have at least 1GB of RAM. If soritng videos you may need significantly more RAM (4GB, 8GB depending on the size of the video file)

Sorted video files seem to be significantly larger in memory than the unsorted version of the video (6.7MB -> 70MB). This may be fixable by tweaking some OpenCV settings but will need to look into it.

As of now a video file can only be sorted in one go, start to finish. If you computer shuts off, goes to sleep, or for any reason any issue occurs during the sorting, the whole new video file will be corrupted and the sorting process will need to restart (the original file will not be corrupted)
In the future I would like to add the feature of sorting the video in segments and stitching the together at the end so if any issue occurs it will be contained to the current segment.

### Remaining items before initial release
  * Filter image and video folders
  * Determine when video/image formats can be supported
  * Allow saving of images and videos to desired ouput format
  * Allow customization on max window/image display size
  * Refine UI and reduce space taken
  * Write documentation and create walkthrough video
  * Include default images/videos for testing
  * Improve performance of mask brush and implement Click and Drag brush mode


### Todo List

* Enhance masking functionality
  * Use mask as intervals
  * Improve performance of mask drawing
  * Add ability to use any image as a mask with threshold customization
  * Write fragment shader to display what intervals will be sorted, dynamically as thresholds are modified
  * Add color picker to modify mask color (default of white is not visible on white portions of images)
* Improve UI clarity and usability: Redo UI in [ofxDatGui](https://github.com/braitsch/ofxDatGui)
  * Add diagnostics and metrics to be shown in app (time taken, current frame for videos, what the application is currently doing, error messages, tooltips)
  * Add ability to apply curves or easing functions to parameter values to change them over the course of a video sorting
  * Allow for dynamic reload of image/video load buttons when contents of folder change
* Add undo/redo functionality

### Examples

![](media/MultiSort.gif)
![](media/SortingOptions.gif)
![](media/SortedVideo.gif)