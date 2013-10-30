/*
 * Guppy.hpp
 *
 *  Created on: Oct 30, 2013
 *      Author: grewe
 */

#ifndef GUPPY_HPP_
#define GUPPY_HPP_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

class Guppy {

private:
	VideoCapture cap;

public:
	Guppy(int cam_no);

	bool isOpened() const;

	bool getFrame(Mat &frame);

	void closeCam();

	void exposure(double exposure);
	double exposure();

	void dimensions(double width=752.0, double height=580.0);
	double frameWidth();
	double frameHeight();

	virtual ~Guppy();
};

#endif /* GUPPY_HPP_ */
