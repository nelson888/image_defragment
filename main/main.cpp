#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace cv;

const int LOADING_TYPE = CV_LOAD_IMAGE_UNCHANGED;
const int BIG_AUGMENTATION = 500;


Mat rotatedFragment(Mat fragment, Point2f center, float angle) {
  Mat rotationMatrix = getRotationMatrix2D(center, angle, 1.0);
	Mat result;
	warpAffine(fragment, result, rotationMatrix, fragment.size());
	return result;
}

void putFragment(Mat imageOut, Mat fragment, int x, int y, float angle) {
  Point2f rotationCenter(fragment.cols * 0.5f, fragment.rows * 0.5f);
  fragment = rotatedFragment(fragment, rotationCenter, angle);
  Rect area(x - fragment.cols / 2 + BIG_AUGMENTATION * 0.5f, y - fragment.rows / 2 + BIG_AUGMENTATION * 0.5f, fragment.cols, fragment.rows);
  vector<Mat> channels;
  split(fragment, channels);
                                 // we pass the alpha channel as mask because we only copy visible elements (0 means no visibility)
  fragment.copyTo(imageOut(area), channels[3]);
}

/**
* Image should be passed as first argument
* (optional) second argument should be 's' if you want to save the image instead of displaying it
**/
int main(int argc, char** argv){
	Mat imageIn = imread( argv[1], LOADING_TYPE );
	if(!imageIn.data) {
    cout <<  "Could not open or find the image" << std::endl;
    return -1;
  }

	// create empty image bigger than the original (to avoid problems when writting rotated images)
	Mat bigImageOut(imageIn.rows + BIG_AUGMENTATION, imageIn.cols + BIG_AUGMENTATION, CV_8UC4);
  bigImageOut = Scalar(255, 255, 255, 0); //make transparent background


  // reads fragments.txt
	ifstream infile("../fragments.txt");
	string line;
	while (getline(infile, line)) {
		istringstream iss(line);
		int id, posX, posY;
		float angle;
		if(!(iss >> id >> posX >> posY >> angle)) {
		  std::cout << "Error while parsing line: '" << line << "'" << endl;
		  continue; 
		}
		Mat fragment = imread("../frag_eroded/frag_eroded_"+ to_string(id) +".png", IMREAD_UNCHANGED);
		putFragment(bigImageOut, fragment, posX, posY, angle);
	}

  // Crop the big image to have the correct size (Region Of Interest)
  Rect regionOfInterest(BIG_AUGMENTATION * 0.5f, BIG_AUGMENTATION * 0.5f, imageIn.cols, imageIn.rows);
  Mat imageOut = bigImageOut(regionOfInterest);
	if (argc > 2 && argv[2][0] == 's') {
	  imwrite("./result.png", imageOut);
	  cout << "Saved image to result.png" << endl;
	} else {
	  imshow( "Display window", imageOut);
	  waitKey(0); 
	}	
	return 0;
}

