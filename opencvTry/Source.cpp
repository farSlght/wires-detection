#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat src, mask;
vector<Vec4i> lines;
Point mouse_position;


int count_av_color(int x, int y);
int backgr_check(int num);
void mouse_callback(int event, int x, int y, int flags, void* userdata);
bool submit_wires(Point line_begin, Point line_end);


int main(int argc, char** argv) {


	src = imread("1.jpg", IMREAD_COLOR);
	Mat src1 = imread("1.jpg", IMREAD_COLOR);
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

	Mat inpaintMask = Mat::zeros(src.size(), CV_8U);
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

	/// A base algorithm, naturally should be called right after backgr_check
	namedWindow("Please submit wires to delete", WINDOW_KEEPRATIO);
	imshow("Please submit wires to delete", mask);
	waitKey(0);
	setMouseCallback("Please submit wires to delete", mouse_callback, NULL);
	waitKey(0);
	for (size_t i = 0; i < lines.size(); i++)
	{
		submit_wires(Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]));
	}




	namedWindow("Hough", WINDOW_KEEPRATIO);
	imshow("Hough", src);

	waitKey(0);

	inpaint(src1, inpaintMask, src1, 5, INPAINT_TELEA);

	namedWindow("r", WINDOW_KEEPRATIO);
	imshow("r", src1);

	waitKey(0);







	return 0;
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
	res = count_av_color(d.x, d.y) + count_av_color(c.x, c.y) + count_av_color(e.x, e.y);

	if (res > 1)
		return 0;
	else
		return -1;
}

int count_av_color(int x, int y)
{
	//mask - one line we are working with
	Mat init = src = imread("1.jpg", IMREAD_COLOR);

	Mat elem = getStructuringElement(MORPH_RECT, Size(1, 1), Point(-1, -1));
	Mat dilated;
	dilate(mask, dilated, elem);

	//storing two massives of colors
	Point3i m1(0, 0, 0), m2(0, 0, 0);
	int m1Count = 0, m2Count = 0;

	int kernel = 3;

	bool line = false, change_diff = false, change_same = false, first_array = true;

	int i, j;

	for (j = y - kernel; j <= y + kernel; j++)
	{
		if (j % 2 == 0)
		{
			for (i = x - kernel; i <= x + kernel; i++)
			{
				if ((int)dilated.at<uchar>(j, i) == 0 && line &&
					(change_diff || !(change_diff || change_same)))
				{
					change_diff = false;
					change_same = false;
					if (first_array)
						first_array = false;
					else
						first_array = true;
				}
				if ((int)dilated.at<uchar>(j, i) == 0)
					line = false;
				else line = true;

				if (!line)
				{
					change_same = false;
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
			i--;
		}
		else
		{
			for (i = x + kernel; i >= x - kernel; i--)
			{
				if ((int)dilated.at<uchar>(j, i) == 0 && line &&
					(change_diff || !(change_diff || change_same)))
				{
					change_diff = false;
					change_same = false;
					if (first_array)
						first_array = false;
					else
						first_array = true;
				}
				if ((int)dilated.at<uchar>(j, i) == 0)
					line = false;
				else line = true;

				if (!line)
				{
					change_same = false;
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
			i++;
		}
		change_same = false;
		change_diff = false;

		if (((int)dilated.at<uchar>(j + 1, i) != 0 && line) || ((int)dilated.at<uchar>(j + 1, i) == 0 && !line))
			change_same = true;
		else
			change_diff = true;
	}

	m1.x /= m1Count;
	m1.y /= m1Count;
	m1.z /= m1Count;

	m2.x /= m2Count;
	m2.y /= m2Count;
	m2.z /= m2Count;

	//add checking with threshold

	return 1;
}



void mouse_callback(int event, int x, int y, int flags, void* userdata) {
	if (flags == (EVENT_RBUTTONDOWN)) {
		mouse_position.x = x;
		mouse_position.y = y;
	}
}

bool submit_wires(Point line_begin, Point line_end) {

	if ((line_begin.x < 0) ||
		(line_begin.x > mask.rows) ||
		(line_begin.y < 0) ||
		(line_begin.y > mask.cols)){
		cout << "__________________";
		cout << "_________OUT OF IMAGE RANGE___________";
		cout << "__________________";
		system("pause");
	}

	if ((line_end.x < 0) ||
		(line_end.x > mask.rows) ||
		(line_end.y < 0) ||
		(line_end.y > mask.cols)){
		cout << "__________________";
		cout << "_________OUT OF IMAGE RANGE___________";
		cout << "__________________";
		system("pause");
	}

	int eps = 5;
	for (int i = line_begin.y - eps; i < line_begin.y + eps; ++i){
		for (int j = line_begin.x - eps; j < line_begin.x + eps; ++j) {

			if ((j <= 0) ||
				(j >= mask.cols - (eps * 2)) ||
				(i <= 0) ||
				(i >= mask.rows - (eps * 2))){
				cout << "__________________";
				cout << "OUT OF IMAGE RANGE";
				cout << "__________________";
				continue;
			}

			cout << "i=" << i << " j=" << j << "\nmouse_position_x=" << mouse_position.x << " mouse_position_y=" << mouse_position.y << "\nline_begin_y=" << line_begin.y << " line_begin_x" << line_begin.x << endl;
			mask.at<uchar>(i, j) = 200; /// check which pixel is operated
			imshow("Please submit wires to delete", mask);
			waitKey(1);

			if ((i == mouse_position.y) &&
				(j == mouse_position.x))
				return true;
		}
	}
}