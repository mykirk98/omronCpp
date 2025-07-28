#pragma once
#include <opencv2/opencv.hpp>
#include <STApi_TL.h>

using namespace StApi;
using namespace cv;

class ImageProcess
{
public:
	static Mat ConvertToMat(IStImage* pImage);
};

