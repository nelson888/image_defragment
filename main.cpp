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

void putFragment(Mat* imageOut, Mat* fragment, int x, int y, float angle) {
 
  // TODO rotate fragment
  Rect area(x, y, fragment->cols, fragment->rows);
  fragment->copyTo((*imageOut)(area));
}

int main(int argc, char** argv){
	Mat imageIn = imread( argv[1], LOADING_TYPE );
	if(!imageIn.data) {
    cout <<  "Could not open or find the image" << std::endl;
    return -1;
  }

	// create empty image
	Mat imageOut(imageIn.rows, imageIn.cols, 0);
  
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
		putFragment(&imageOut, &fragment, posX, posY, angle);
	}
	
	imshow( "Display window", imageOut );             
	waitKey(0);       	
	
	return 0;
}

