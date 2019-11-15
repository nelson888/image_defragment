#include "opencv2/opencv.hpp"
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char const *argv[])
{
    if (argc != 1) {
        string imageName(argv[1]);
        Mat imageIN = imread("./images/"+imageName, CV_LOAD_IMAGE_UNCHANGED);
        Mat imageProcessing;
        GaussianBlur(imageIN, imageProcessing, Size(5,5), 25, 0);
        imshow("My blurred image",imageProcessing);
    }
    return 0;
}
