#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>


struct Fragment {
  int x;
  int y;
  float angle;
};

std::map<int, Fragment> getSolutionFragments() {
  std::map<int, Fragment> fragments;
  std::ifstream infile("solution.txt");
	std::string line;
	while (getline(infile, line)) {
		std::istringstream iss(line);
		int id, posX, posY;
		float angle;
		if(!(iss >> id >> posX >> posY >> angle)) {
		  std::cout << "Error while parsing line: '" << line << "'" << std::endl;
		  continue; 
		}
		fragments[id] = {posX, posY, angle};
	}
  return fragments;
}

int getArea(int fragmentId) {
  Mat img = imread("./frag_eroded/frag_eroded_"+ to_string(id) +".png");
  return img.cols * img.rows;
}

int main(int argc, char** argv) {
	if(argc != 4) {
    std::cout <<  "You must provide Dx Dy Dalpha" << std::endl;
    return -1;
  }
  
  int dX = std::stoi(argv[1]);
  int dY = std::stoi(argv[2]);
  int dAlpha = std::stoi(argv[3]);
  
  int rightLocatedSurface = 0;
  int badLocatedSurface = 0;
  
  std::map<int, Fragment> solutionFragments = getSolutionFragments();
  
  // reads fragments.txt
  std::ifstream infile("fragments.txt");
	std::string line;
	while (getline(infile, line)) {
		std::istringstream iss(line);
		int id, posX, posY;
		float angle;
		if(!(iss >> id >> posX >> posY >> angle)) {
		  std::cout << "Error while parsing line: '" << line << "'" << std::endl;
		  continue; 
		}
		// compare with the solution
		Fragment fragment = {posX, posY, angle};
		Fragment solFragment = solutionFragments[id];
		int area = getArea(id);
		
    if (std::abs(fragment.x - solFragment.x) > dX ||
        std::abs(fragment.y - solFragment.y) > dY ||
        std::abs(fragment.angle - solFragment.angle) > dAlpha) {
      badLocatedSurface += area;
    } else {
      rightLocatedSurface += area;
    }
	}
	
	std::cout << "Right Located surface: " << rightLocatedSurface;
	std::cout << "Bad Located surface: " << badLocatedSurface;
	std::cout << "Precision: " << rightLocatedSurface - badLocatedSurface / (rightLocatedSurface + badLocatedSurface);
   	
	return 0;
}

