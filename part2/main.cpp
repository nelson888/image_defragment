#include "opencv2/opencv.hpp"
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char const *argv[])
{
    if (argc == 5) {
        string imageName(argv[1]);
        Mat imageIN = imread("./images/"+imageName, CV_LOAD_IMAGE_UNCHANGED);
        Mat imageProcessing(imageIN.rows, imageIN.cols, CV_8UC4);
        int g_kSize_Rows = stoi(argv[2]);
        int g_kSize_Cols = stoi(argv[3]);
        Size kernelSize(g_kSize_Rows, g_kSize_Cols);
        double g_sigmaX = stod(argv[4]);
        GaussianBlur(imageIN, imageProcessing, kernelSize, g_sigmaX, 0);
        imshow("My blurred image",imageProcessing);
        waitKey(0);
    }
    else{
        cout << "Syntax is : ./detectionCercle [image] [kernel size rows] [kernel size colums] [sigmaX]" << endl;
    }
    return 0;
}
