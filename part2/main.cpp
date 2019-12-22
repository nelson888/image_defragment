#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include <math.h>
#include <vector>
#include<list>

using namespace std;
using namespace cv;


struct Circle {
  int r;
  int c;
  int rad;
};

double getDoubleInput(){
    string s_double;
    cin >> s_double;
    return stod(s_double);
}

int getIntInput(){
    int i;
    cin >> i;
    return i;
}

string getStringInput(){
    string str; 
    cin >> str;
    return str;
}

void getGaussianBlurParameters(Size* kSize, double* g_SigmaX){
    cout << "Récupération des paramètres pour le filtrage Gaussien." << endl;
    cout << "Entrez le nombre de ligne du noyau : ";
    kSize->height = getIntInput();
    cout << "Entrez le nombre de colonne du noyau : ";
    kSize->width = getIntInput();
    cout << "Entrez la déviation standard en X : ";
    *g_SigmaX = getDoubleInput();
}

Mat applyGaussianFilter(Mat imageInput, Size kSize, double g_sigmaX){
    Mat imageBlurred;
    GaussianBlur(imageInput, imageBlurred, kSize, g_sigmaX);
    return imageBlurred;
}

Mat applySobel(Mat source){
    cout << "Applying Sobel" << endl;

    Mat srcGray;
    cvtColor(source, srcGray, CV_BGR2GRAY);

    Mat grad_X, grad_Y;
    Sobel(srcGray, grad_X, CV_16S, 1, 0);
    Sobel(srcGray, grad_Y, CV_16S, 0, 1);
    Mat edges;
    
    int cannyThreshold = 100;
    Canny(grad_X, grad_Y, edges, cannyThreshold, cannyThreshold*3, false);
    edges.convertTo(edges,CV_8U);
    return edges;
}

double distance(int i1, int j1, int i2, int j2) {
  int i = i1 - i2;
  int j = j1 - j2;
  return sqrt(i * i + j * j); 
}


constexpr int MAX_RADIUS = 100;
constexpr int BORDER_THRESHOLD = 250; // color threshold


// local maximum 
void addMaximum(vector<vector<vector<uint>>> *acc, vector<Circle> *maximums, int i, int j, int k, int rows, int cols, int nbRads) {
  int value = (*acc)[i][j][k];
  
  // the neighbours are beetween [(i - 1; i + 1)][(j - 1; j + 1)][(k - 1; k + 1)]
  for(int r = i - 1; r <= i + 1 && r < rows; r++) {
    if (r < 0) continue;
    for(int c = j - 1; c <= j + 1 && c < cols; c++) {
      if (c < 0) continue;
      for(int rad = k - 1; rad <= k + 1 && rad < nbRads; rad++) {
        if (rad < 0) continue;
        if ((*acc)[r][c][rad] > value) return;
      }
    }
  }
  Circle c ={i, j, k};
  maximums->push_back(c);
}

void computeCircles(Mat* img, Mat* dest, int N) {
  std::cout << "Computing circles" << std::endl;
  // initialize accumulator 
  int rows = img->rows;
  int cols = img->cols;
  int nbRads = 2 * std::min(rows, cols);
  vector<vector<vector<uint>>> acc (rows, vector<vector<uint>>(cols, vector<uint>(nbRads, 0)));

  for (int i=0; i<rows; i++) {
    for (int j=0; j<cols; j++) {
      for (int k=0; k<nbRads; k++) {
        acc[i][j][k] = 0;
      }
    }
  }

  // compute circles
  for(int y = 0; y < img->rows; y++){
    for (int x = 0; x < img->cols; x++){
      if (img->at<uchar>(y,x) > BORDER_THRESHOLD) {
        for(int j=0; j < rows; j++) {
          for (int i=0; i < cols; i++) {
            double d = distance(i, j, y, x);
            acc[j][i][(int)(d)]+= img->at<uchar>(y,x); // magnitude
          }
        }
      }
    }
  }
  
  // compute local max
  std::cout << "Computing local maximums" << std::endl;
  vector<Circle> maximums = {};
  for (int i=0; i<rows; i++) {
    for (int j=0; j<cols; j++) {
      for (int k=0; k<nbRads; k++) {
        addMaximum(&acc, &maximums, i, j, k, rows, cols, nbRads);
      }
    }
  }
  
  std::cout << "Getting " << N << " maximums" << std::endl;
  std::sort(maximums.begin(), maximums.end(), [&acc](Circle a, Circle b) {
            return (acc[a.r][a.c][a.rad] < acc[b.r][b.c][b.rad]);
        });
  

  
  vector<Circle> circles = {};
  for(int i = maximums.size() - N; i < maximums.size(); i++) {  
    circles.push_back(maximums[i]);
  }
  for (Circle c : circles) {
    std::cout << "Circle r:" << c.r << " c:" << c.c << " rad:" << c.rad << endl;
    circle((*dest), Point(c.r, c.c), c.rad, Scalar(0,0, 255));
  }
  
}


double getSeconds(int64 endTicks, int64 startTicks) {
 return (endTicks - startTicks) / getTickFrequency();
}


int main(int argc, char const *argv[])
{
    if (argc != 4)
    {        
        cout << "Syntax is : ./detectionCercle [image] [gaussian blur: y/n] [N] (with N the number of circles to detect)" << endl;
        return 1;
    }
    string imageName(argv[1]);
    Mat imageGray = imread(imageName, IMREAD_GRAYSCALE);
    Mat imageColored = imread(imageName, IMREAD_COLOR);
    
    Mat imageGaussianBlur;
    Mat imageToProcess;
    Mat imageBorders;
    
    if (*argv[2] == 'y'){
        Size kernelSize(0,0);
        double g_sigmaX;
        getGaussianBlurParameters(&kernelSize, &g_sigmaX);
        imageGaussianBlur = applyGaussianFilter(imageColored, kernelSize, g_sigmaX);
        imageGaussianBlur.copyTo(imageToProcess);
    }
    else{
        imageColored.copyTo(imageToProcess);
    }

    int64 startTickCounts = getTickCount();

    imageBorders = applySobel(imageToProcess);

    computeCircles(&imageBorders, &imageColored, stoi(argv[3]));
    cout << "Computation took " << getSeconds(getTickCount(), startTickCounts) << "s" << endl;

    if(*argv[2] == 'y')
        imshow("Blurred image",imageGaussianBlur);
    imshow("Borders",imageBorders);
    imshow("Cercle detection image",imageColored);
    
    waitKey(0);

    return 0;
}
