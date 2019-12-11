#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

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

void applySobel(Mat* source, Mat* dest){
    cout << "Application du filtrage Sobel" << endl;
    int scale, delta;
    /*
    cout << "Entrez le scale :";
    scale = getIntInput();
    cout << "Entrez de delta :";
    delta = getIntInput();
    */
    Mat imageSourceGray;
    cvtColor(*source,imageSourceGray, CV_BGR2GRAY);
    Mat grad_X, grad_Y;
    cout << "Sobel pour X" << endl;
    Sobel(imageSourceGray, grad_X, CV_32S, 1, 0);
    cout << "Sobel pour Y" << endl;
    Sobel(imageSourceGray, grad_Y, CV_32S, 0, 1);
    cout << "Je passe juste avant le constructeur de la matrice" << endl;
    Mat combination(grad_X.rows,grad_Y.cols, CV_32S);
    for(int i = 0; i < combination.rows; i++){
        cout << "Je suis passé par " << i << " lignes" << endl;
        for (int j = 0; j < combination.cols; j++){
            cout << "Je suis passé par " << j << " colonnes dans la ligne "<< i << endl;
            Point pixel(i,j);
            combination.at<int>(pixel) = sqrt( (grad_X.at<int>(pixel)*grad_X.at<int>(pixel)) + (grad_Y.at<int>(pixel)*grad_Y.at<int>(pixel)) );
        }
    }

    combination.convertTo(*dest, CV_8U);
}

int main(int argc, char const *argv[])
{
    if (argc == 3) {
        string imageName(argv[1]);
        Mat imageIN = imread("./images/"+imageName, CV_LOAD_IMAGE_GRAYSCALE);
        Mat imageProcessing(imageIN.rows, imageIN.cols, CV_8U);
        if (*argv[2] == 'y'){
            Size kernelSize(0,0);
            double g_sigmaX;
            getGaussianBlurParameters(&kernelSize, &g_sigmaX);
            GaussianBlur(imageIN, imageProcessing, kernelSize, g_sigmaX);
        }
        applySobel(&imageIN, &imageProcessing);
        imshow("My blurred image",imageProcessing);
        waitKey(0);
    }
    else{
        cout << "Syntax is : ./detectionCercle [image] [gaussian blur: y/n]" << endl;
    }
    return 0;
}