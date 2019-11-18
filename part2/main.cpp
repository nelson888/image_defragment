#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

class CircleDetection
{
private:
    
public:
    
    CircleDetection(/* args */);
    ~CircleDetection();
};

CircleDetection::CircleDetection(/* args */)
{
}

CircleDetection::~CircleDetection()
{
}

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

void getSobelParameters()

int main(int argc, char const *argv[])
{
    if (argc == 3) {
        string imageName(argv[1]);
        Mat imageIN = imread("./images/"+imageName, CV_LOAD_IMAGE_UNCHANGED);
        Mat imageProcessing(imageIN.rows, imageIN.cols, CV_8UC4);
        if (*argv[2] == 'y'){
            Size kernelSize(0,0);
            double g_sigmaX;
            getGaussianBlurParameters(&kernelSize, &g_sigmaX);
            GaussianBlur(imageIN, imageProcessing, kernelSize, g_sigmaX);
        }        
        imshow("My blurred image",imageProcessing);
        waitKey(0);
    }
    else{
        cout << "Syntax is : ./detectionCercle [image] [gaussian blur: y/n]" << endl;
    }
    return 0;
}










