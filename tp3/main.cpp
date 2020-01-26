#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;

const int LOADING_TYPE = CV_LOAD_IMAGE_UNCHANGED;

void getKeyPoints(Mat image, Mat fragment, std::vector<KeyPoint>* keypoints_image,
        std::vector<KeyPoint>* keypoints_fragments,
                  std::vector< DMatch>* good_matches) {
    //-- Step 1: Detect the keypoints using SURF Detector
    Ptr<FeatureDetector> detector = ORB::create();

    detector->detect(fragment, *keypoints_fragments);
    detector->detect(image, *keypoints_image);

    //-- Step 2: Calculate descriptors (feature vectors)
    Ptr<DescriptorExtractor> extractor = ORB::create();

    Mat descriptors_object, descriptors_scene;

    extractor->compute(fragment, *keypoints_fragments, descriptors_object);
    extractor->compute(image, *keypoints_image, descriptors_scene);

    //-- Step 3: Matching descriptor vectors using FLANN matcher
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce");
    std::vector< DMatch > matches;
    matcher->match(descriptors_object, descriptors_scene, matches);

    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for (int i = 0; i < descriptors_object.rows; i++)
    {
        double dist = matches[i].distance;
        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }

    printf("-- Max dist : %f \n", max_dist);
    printf("-- Min dist : %f \n", min_dist);

    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )


    for (int i = 0; i < descriptors_object.rows; i++) {
        if (matches[i].distance < 3 * min_dist) {
            good_matches->push_back(matches[i]);
        }
    }

    std::cout << "Found " << good_matches->size() << " good matches" << std::endl;
    for (auto match: *good_matches) {
        std::cout << "Found match with distance: " << match.distance
        << " from " << (*keypoints_image)[match.queryIdx].pt << "to "
        << (*keypoints_fragments)[match.queryIdx].pt
        << " with an angle of " << (*keypoints_fragments)[match.queryIdx].angle << std::endl;
    }
}
int main(int argc, char** argv)
{

    Mat image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    Mat fragment = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);


    if (!fragment.data || !image.data)
    {
        std::cout << " --(!) Error reading images " << std::endl; return -1;
    }

    std::vector<KeyPoint> keypoints_image;
    std::vector<KeyPoint> keypoints_fragments;
    std::vector< DMatch> good_matches;

    getKeyPoints(image, fragment, &keypoints_image, &keypoints_fragments,
                 &good_matches);

    Mat img_matches;

    drawMatches(fragment, keypoints_fragments, image, keypoints_image, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for (int i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints_fragments[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints_image[good_matches[i].trainIdx].pt);
    }

   
    Mat H;
    
    try {
      H = findHomography(obj, scene, CV_RANSAC);
    } catch (const std::exception& e) {
      std::cout << "Error while finding homography" << std::endl;
    } 

    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(fragment.cols, 0);
    obj_corners[2] = cvPoint(fragment.cols, fragment.rows); obj_corners[3] = cvPoint(0, fragment.rows);
    std::vector<Point2f> scene_corners(4);

    perspectiveTransform(obj_corners, scene_corners, H);

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    line(img_matches, scene_corners[0] + Point2f(fragment.cols, 0), scene_corners[1] + Point2f(fragment.cols, 0), Scalar(0, 255, 0), 4);
    line(img_matches, scene_corners[1] + Point2f(fragment.cols, 0), scene_corners[2] + Point2f(fragment.cols, 0), Scalar(0, 255, 0), 4);
    line(img_matches, scene_corners[2] + Point2f(fragment.cols, 0), scene_corners[3] + Point2f(fragment.cols, 0), Scalar(0, 255, 0), 4);
    line(img_matches, scene_corners[3] + Point2f(fragment.cols, 0), scene_corners[0] + Point2f(fragment.cols, 0), Scalar(0, 255, 0), 4);

    //-- Show detected matches
    imshow("Good Matches & Object detection", img_matches);
    imwrite( "./result.jpg", img_matches );

    waitKey(0);
    return 0;
}
