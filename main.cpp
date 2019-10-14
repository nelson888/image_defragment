#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/** objectifs: 2 binaires:
 * 1) binaire qui calcule a partir des fragments l'image reconstruit
 * 2) binaire qui a le fichier de position des fragments attendu (fichier du prof)
 * et un autre fichier qui correspond aux position des fragments qu'on a eu

**/

using namespace std;
using namespace cv;

const int LOADING_TYPE = CV_LOAD_IMAGE_GRAYSCALE;
const int AUGMENTED_COLS = 500;
const int AUGMENTED_ROWS = 500;
/*
void rotate(Mat* fragment, Point2f center, float angle) {
  Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
  // determine bounding rectangle, center not relevant
  Rect2f bbox = cv::RotatedRect(Point2f(), fragment->size(), angle).boundingRect2f();
  // adjust transformation matrix
  rot.at<double>(0,2) += bbox.width/2.0 - fragment->cols/2.0;
  rot.at<double>(1,2) += bbox.height/2.0 - fragment->rows/2.0;

  Mat dst;
  warpAffine((*fragment), dst, rot, bbox.size());
  (*fragment) = dst;  

}
*/

Mat rotatedFragment(Mat* fragment, Point2f center, float angle) {
  Mat rotationMatrix = getRotationMatrix2D(center, angle, 1.0);
	Mat result;
	warpAffine((*fragment), result, rotationMatrix, fragment->size());
	return result;
}

void putFragment(Mat* imageOut, Mat* fragment, int x, int y, float angle) {
  Rect area(x + AUGMENTED_COLS / 2.0f, y + AUGMENTED_ROWS / 2.0f, fragment->cols, fragment->rows);
  Point2f rotationCenter(fragment->cols / 2.0f, fragment->rows / 2.0f);
  rotatedFragment(fragment, rotationCenter, angle).copyTo((*imageOut)(area));
}

int main(int argc, char** argv){
	Mat imageIn = imread( argv[1], LOADING_TYPE );
	if(!imageIn.data) {
    cout <<  "Could not open or find the image" << std::endl;
    return -1;
  }

	// create empty image bigger than the original (to avoid problems when writting rotated images)
	Mat bigImageOut(imageIn.rows + AUGMENTED_ROWS, imageIn.cols + AUGMENTED_COLS, 0);

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
  Rect regionOfInterest(AUGMENTED_COLS / 2.0f, AUGMENTED_ROWS / 2.0f, imageIn.cols, imageIn.rows);
  Mat imageOut = bigImageOut(regionOfInterest);
	//imshow( "Display window", imageOut);   
	imwrite("./result.png", imageOut);          
	waitKey(0);       	
	
	std::cout << "IN : " << imageIn.rows << ' ' << imageIn.cols << endl;
	std::cout << "BIG: " << bigImageOut.rows << ' ' << bigImageOut.cols << endl;
	std::cout << "RES: " << imageOut.rows << ' ' << imageOut.cols << endl;
	return 0;
}

