#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat src, mask;
vector<Vec4i> lines;




int cont_av_color(int x, int y) 
{
	//mask - one line we are working with
	Mat init = src = imread("1.jpg", IMREAD_COLOR);

	Mat elem = getStructuringElement(MORPH_RECT, Size(1, 1), Point(-1, -1));
	Mat dilated;
	dilate(mask, dilated, elem);

	//storing two massives of colors
	Point3i m1 (0, 0, 0), m2 (0, 0, 0);
	int m1Count = 0, m2Count = 0;
	//uchar u;
	int kernel = 3;

	bool line = false, change = false, first_array = true; 

	//int value = (int)dilated.at<uchar>(553, 458);


	for (int j = y - kernel; j <= y + kernel; j++)
	{
		if (j % 2 == 0)
		{
			for (int i = x - kernel; i <= x + kernel; i++)
			{
				
				if (dilated.at<uchar>(i, j) == 0 && line && !change)
				{
					if (first_array)
						first_array = false;
					else 
						first_array = true;
				}
				
				change = false;
				
				//int value = (int)dilated.at<uchar>(j, i);



				if ((int) dilated.at<uchar>(j, i) == 0)
					line = false;
				else line = true;

				if (!line)
				{
					if (first_array)
					{
						m1.x += (int) src.at<Vec3b>(Point(i, j))[0];
						m1.y += (int) src.at<Vec3b>(Point(i, j))[1];
						m1.z += (int) src.at<Vec3b>(Point(i, j))[2];
						m1Count++;
					}
					else
					{
						m2.x += (int) src.at<Vec3b>(Point(i, j))[0];
						m2.y += (int) src.at<Vec3b>(Point(i, j))[1];
						m2.z += (int) src.at<Vec3b>(Point(i, j))[2];
						m2Count++;
					}
				}
			}
		}
		else
		{
			for (int i = x + kernel; i >= x - kernel; i--)
			{
				if (dilated.at<uchar>(j, i) == 0 && line && !change)
				{
					if (first_array)
						first_array = false;
					else
						first_array = true;
				}

				change = false;

				if (dilated.at<uchar>(j, i) == 0)
					line = false;
				else line = true;

				if (!line)
				{
					if (first_array)
					{
						m1.x += (int)src.at<Vec3b>(Point(i, j))[0];
						m1.y += (int)src.at<Vec3b>(Point(i, j))[1];
						m1.z += (int)src.at<Vec3b>(Point(i, j))[2];
						m1Count++;
					}
					else
					{
						m2.x += (int)src.at<Vec3b>(Point(i, j))[0];
						m2.y += (int)src.at<Vec3b>(Point(i, j))[1];
						m2.z += (int)src.at<Vec3b>(Point(i, j))[2];
						m2Count++;
					}
				}



			}
		}
		change = true;
	}

	m1.x /= m1Count;
	m1.y /= m1Count;
	m1.z /= m1Count;

	m2.x /= m2Count;
	m2.y /= m2Count;
	m2.z /= m2Count;



	return 1;
}


int backgr_check(int num)
{
	Point a, b, c;
	a.x = lines[num][0];
	a.y = lines[num][1];
	b.x = lines[num][2];
	b.y = lines[num][3];

	//define center of the line segment
	c.x = (a.x + b.x) / 2;
	c.y = (a.y + b.y) / 2;

	//define to additional points for checking
	Point d, e;
	int lambda = 5; 

	d.x = (b.x + lambda*a.x) / (1 + lambda);
	d.y = (b.y + lambda*a.y) / (1 + lambda);
	e.x = (a.x + lambda*b.x) / (1 + lambda);
	e.y = (a.y + lambda*b.y) / (1 + lambda);

	int res = 0;
	res = cont_av_color(d.x, d.y) + cont_av_color(c.x, c.y) + cont_av_color(e.x, e.y);

	if (res > 1)
		return 0;
	else 
		return -1;
}


int main(int argc, char** argv) {
	
	
	src = imread("1.jpg", IMREAD_COLOR);
	if (src.empty())
	{
		cerr << "No image supplied" << endl;
		return -1;
	}
	GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
	namedWindow("Initial photo", WINDOW_KEEPRATIO);
	imshow("Initial photo", src);
	waitKey(0);
	
	Mat src_closed, result_gray, binary, binary1;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
	//morphologyEx(src, src_closed, MORPH_CLOSE, element);

	Mat result, result_gray_blur;

	morphologyEx(src, result, MORPH_BLACKHAT, element);

 	//result = src_closed - src;

	namedWindow("result", WINDOW_KEEPRATIO);
	imshow("result", result);
	waitKey(0);

	cvtColor(result, result_gray, COLOR_BGR2GRAY);

	GaussianBlur(result_gray, result_gray_blur, Size(3, 3), 0, 0, BORDER_DEFAULT);
	//GaussianBlur(result_gray_blur, result_gray_blur, Size(3, 3), 0, 0, BORDER_DEFAULT);

	threshold(result_gray_blur, binary1, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	threshold(result_gray, binary, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	namedWindow("Binary photo", WINDOW_KEEPRATIO);
	imshow("Binary photo", binary);

	namedWindow("Binary photo + Gaussian blur", WINDOW_KEEPRATIO);
	imshow("Binary photo + Gaussian blur", binary1);

	//medianBlur(binary1, binary1, 3);

	waitKey(0);

	Mat inpaintMask = Mat :: zeros(src.size(), CV_8U);
	Mat res;


	
	HoughLinesP(binary1, lines, 1, CV_PI / 180, 100, 30, 4);
	for (size_t i = 0; i < lines.size(); i++)
	{
		mask = Mat::zeros(src.size(), CV_8U);
		line(mask, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(255, 255, 255), 1, CV_AA);
		if (!backgr_check(i))
		{
			line(src, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0, 0, 255), 1, CV_AA);
			line(inpaintMask, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(255, 255, 255), 1, CV_AA);
		}
		
		
	}

	namedWindow("M", WINDOW_KEEPRATIO);
	imshow("M", mask);



	namedWindow("Hough", WINDOW_KEEPRATIO);
	imshow("Hough", src);

	waitKey(0);

	
	 
	

	

	return 0;
}
