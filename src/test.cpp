//============================================================================
// Name        : guppy.cpp
// Author      : Jan Grewe
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include "Guppy.hpp"

using namespace std;
using namespace cv;


int main() {
	Guppy cam(0);
	cam.exposure(250);
	if (!cam.isOpened())  // if not success, exit program
	{
		cout << "Cannot open camera!" << endl;
		return -1;
	}

	namedWindow("MyVideo",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
	int i = 0;
	while (i < 250)
	{
		Mat frame;
		bool bSuccess = cam.getFrame(frame); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from camera!!" << endl;
			break;
		}
		imshow("MyVideo", frame); //show the frame in "MyVideo" window
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
		i++;
	}
	cam.closeCam();
	return 0;
}
