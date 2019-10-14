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


void putFragment(Mat* imageOut, Mat* fragment, int x, int y, float angle) {

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
  
  // TODO read the fragments.txt and for each line, extract the id, posX, posY, angle
  //                                                then read the image 'frag_eroded/frag_eroded_{id}.png'
  //                                                then call putFragment putFragment(&imageIn, &imageOut, posX, posY, angle)
  
	ifstream infile("fragments.txt");
	string line;
	while (getline(infile, line){
		istringstream iss(line);
		int id, posX, posY;
		float angle;
		if( !(iss >> id >> posX >> posY >> angle)){break;}
		Mat fragment = imread("frag_eroded/frag_eroded_"+id+".png");
		putFragment(&fragment, &imageOut, posX, posY, angle);
	}
	
	imshow( "Display window", imageOut );             
	waitKey(0);       	

	
	return 0;
}

