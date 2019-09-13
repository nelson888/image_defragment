#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <string>

/** objectifs: 2 binaires:
 * 1) binaire qui calcule a partir des fragments l'image reconstruit
 * 2) binaire qui a le fichier de position des fragments attendu (fichier du prof)
 * et un autre fichier qui correspond aux position des fragments qu'on a eu

**/

using namespace std;
using namespace cv;

double thresh = 100;

void processFragment(Mat imageIn, Mat imageOut, Mat fragment) {
		//TODO
		// finding contours
		Mat canny_output;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		
		// Detect edges using canny
		Canny( imageIn, canny_output, thresh, thresh*2, 3 );
		// Find contours
		findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

		std::cout << contours.size() << endl;
}


int main(int argc, char** argv){



	Mat imageIn = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	
	int nbFragments = 1; // TODO parse from command line argument
	
	if(! imageIn.data )                              // Check for invalid input
    	{
        	cout <<  "Could not open or find the image" << std::endl ;
        	return -1;
   	}
	
	Mat imageOut(imageIn.rows, imageIn.cols, 0);
       
    for (int i=0; i < nbFragments; i++) {
		std::string fileName = "./frag_eroded/frag_eroded_" + std::to_string(i) + ".png";
		Mat fragmentImg = imread(fileName , CV_LOAD_IMAGE_GRAYSCALE );
		std::cout << "Processing " << fileName << std::endl;
		processFragment(imageIn, imageOut, fragmentImg);
	}
	
	imshow( "Display window", imageOut );             
	waitKey(0);       	

	
	return 0;
}

