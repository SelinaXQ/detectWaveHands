/*by Qian Xiang
wave hands
*/
#include <iostream>   
#include <string>   
#include <iomanip>   
#include <sstream>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/core/core.hpp>        
#include <opencv2/highgui/highgui.hpp>  
using namespace cv;
using namespace std;

int main(int arg, const char** argv)
{
	int delay = 1;
	char c;
	int frameNum = -1;          // Frame counter  
	bool lastImgHasHand = false;

	int startX = 0;
	int startY = 0;
	int pt = 0;


	Mat frame;
	Mat frameHSV;
	Mat mask(frame.rows, frame.cols, CV_8UC1);
	Mat dst(frame);


	vector< vector<Point> > contours;   // ÂÖÀª  contours
	vector< vector<Point> > filterContours; // É¸Ñ¡ºóµÄÂÖÀª  filtered contours
	vector< Vec4i > hierarchy;
	vector< Point > hull; // conveixity 

	bool movement = false;
	int count = 0;

	int endX = 0;
	int endY = 0;
	VideoCapture capture;
	capture.open(0);
	if (!capture.isOpened())
	{
		return 0;
	}
	while (1) //Show the image captured in the window and repeat  
	{
		int minX = 320;
		int maxX = 240;
		int minY = 320;
		int maxY = 240;
		capture >> frame;

		if (frame.empty())
		{
			cout << "no frame" << endl;
			break;
		}

		Mat video = frame.clone();

		// Begin  

		medianBlur(frame, frame, 5);
		cvtColor(frame, frameHSV, CV_BGR2HSV);

		Mat dstTemp1(frame.rows, frame.cols, CV_8UC1);
		Mat dstTemp2(frame.rows, frame.cols, CV_8UC1);
		//  The HSV space was quantized to obtain the binary image, and the bright part was the shape of the hand
		inRange(frameHSV, Scalar(0, 30, 30), Scalar(40, 170, 256), dstTemp1);
		inRange(frameHSV, Scalar(156, 30, 30), Scalar(180, 170, 256), dstTemp2);
		bitwise_or(dstTemp1, dstTemp2, mask);

		//  Morphological operation to remove noise and make the boundary of the hand more clear
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		erode(mask, mask, element);
		morphologyEx(mask, mask, MORPH_OPEN, element);
		dilate(mask, mask, element);
		morphologyEx(mask, mask, MORPH_CLOSE, element);
		frame.copyTo(dst, mask);
		contours.clear();
		hierarchy.clear();
		filterContours.clear();
		//  the contours of hand
		findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		for (size_t i = 0; i < contours.size(); i++)
		{

			if (fabs(contourArea(Mat(contours[i]))) > 8000) //  Judge the threshold of hand area
			{
				filterContours.push_back(contours[i]);
			}
		}
		//   draw contours
		if (filterContours.size() > 0)
		{

			count++;
			lastImgHasHand = true;
			drawContours(dst, filterContours, -1, Scalar(255, 0, 255), 3);

			for (size_t j = 0; j < filterContours.size(); j++)
			{
				convexHull(Mat(filterContours[j]), hull, true);
				int hullcount = (int)hull.size();
				for (int i = 0; i < hullcount - 1; i++)
				{
					line(dst, hull[i + 1], hull[i], Scalar(255, 255, 255), 2, CV_AA);
					if (hull[i].x > maxX)
						maxX = hull[i].x;
					if (hull[i].x < minX)
						minX = hull[i].x;
					if (hull[i].y > maxY)
						maxY = hull[i].y;
					if (hull[i].y < minY)
						minY = hull[i].y;

				}

				line(dst, hull[hullcount - 1], hull[0], Scalar(0, 255, 0), 2, CV_AA);

				if (count == 1)// The central position of the first outline exists in the global variable and is compared to the last position.  
				{
					startX = (minX + maxX) / 2;
					startY = (minY + maxY) / 2;

				}
				else
				{
					endX = (minX + maxY) / 2;
					endY = (minY + maxY) / 2;
				}
			}
		}
		else
		{
			if (lastImgHasHand == true)
			{
				if ((startX - endX) < 0)
				{
					pt = 1;
					cout << "wave left" << endl;
				}
				if ((startX - endX) > 0)
				{
					pt = 1;
					cout << "wave right" << endl;
				}
				if ((startY - endY) < 0)
				{
					pt = 1;
					cout << "wave down" << endl;

				}
				if ((startY - endY) > 0)
				{
					pt = 1;
					cout << "wave up" << endl;
				}
				if (pt == 1)
				{
					putText(video, "Wave", Point(30, 70), CV_FONT_HERSHEY_COMPLEX, 2, Scalar(255, 0, 255), 2, 8);
					cvWaitKey(3);

				}
				count = 0;
				lastImgHasHand = false;
			}
		}

		imshow("video", video);
		imshow("result", dst);
		dst.release();
		c = cvWaitKey(1);
		if (c == 27)
			break;
	}
	return 0;
}