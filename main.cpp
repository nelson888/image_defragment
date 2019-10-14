#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace cv;

const int LOADING_TYPE = CV_LOAD_IMAGE_GRAYSCALE;
const int BIG_AUGMENTATION = 500;


Mat rotatedFragment(Mat* fragment, Point2f center, float angle) {
  Mat rotationMatrix = getRotationMatrix2D(center, angle, 1.0);
	Mat result;
	warpAffine((*fragment), result, rotationMatrix, fragment->size());
	return result;
}

void putFragment(Mat* imageOut, Mat* fragment, int x, int y, float angle) {
  Rect area(x + BIG_AUGMENTATION / 2.0f, y + BIG_AUGMENTATION / 2.0f, fragment->cols, fragment->rows);
  Point2f rotationCenter(fragment->cols / 2.0f, fragment->rows / 2.0f);
  rotatedFragment(fragment, rotationCenter, angle).copyTo((*imageOut)(area));
}

/**
* Image should be passed as an argument
**/
int main(int argc, char** argv){
	Mat imageIn = imread( argv[1], LOADING_TYPE );
	if(!imageIn.data) {
    cout <<  "Could not open or find the image" << std::endl;
    return -1;
  }

	// create empty image bigger than the original (to avoid problems when writting rotated images)
	Mat bigImageOut(imageIn.rows + BIG_AUGMENTATION, imageIn.cols + BIG_AUGMENTATION, 0);

  // reads fragments.txt
	ifstream infile("fragments.txt");
	string line;
	while (getline(infile, line)) {
		istringstream iss(line);
		int id, posX, posY;
		float angle;
		if(!(iss >> id >> posX >> posY >> angle)) {
		  std::cout << "Error while parsing line: '" << line << "'" << endl;
		  continue; 
		}
		Mat fragment = imread("./frag_eroded/frag_eroded_"+ to_string(id) +".png", LOADING_TYPE);
		putFragment(&bigImageOut, &fragment, posX, posY, angle);
	}

  // Crop the big image to have the correct size (Region Of Interest)
  Rect regionOfInterest(BIG_AUGMENTATION / 2.0f, BIG_AUGMENTATION / 2.0f, imageIn.cols, imageIn.rows);
  Mat imageOut = bigImageOut(regionOfInterest);
	imshow( "Display window", imageOut);   
	imwrite("./result.png", imageOut);          
	waitKey(0);       	
	return 0;
}

