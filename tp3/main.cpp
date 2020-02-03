#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "opencv2/opencv.hpp"

using namespace cv;

struct AngledPosition {
    bool none; // true if no result was found
    int x;
    int y;
    float angle;
};

const int LOADING_TYPE = CV_LOAD_IMAGE_UNCHANGED;
const int MAX_TRIALS = 8;

const int MIN_POINTS = 2;
const int D = 3; // Number of close data points required to assert that a model fits well to data.

bool vecContains(std::vector<DMatch>* matches, DMatch dMatch) {
    for (auto match: *matches) {
        if (abs(dMatch.distance - match.distance) < 0.2f &&
            dMatch.queryIdx == match.queryIdx &&  dMatch.trainIdx == match.trainIdx) {
            return true;
        }
    }
    return false;
}

std::vector<DMatch> getNRandomValues(int n, std::vector<DMatch>* matches) {
    std::vector<DMatch> result;
    if (matches->size() <= n) {
        for (auto match: *matches) {
            result.push_back(match);
        }
    } else {
        while (result.size() < n) {
            DMatch m = (*matches)[rand() % n];
            if (!vecContains(&result, m)) {
                result.push_back(m);
            }
        }
    }
    return result;
}

bool matchesLargerThanImage(std::vector< DMatch>* matches, std::vector<KeyPoint>* keypoints_image, std::vector<KeyPoint>* keypoints_fragment) {
    for (int i = 0; i < matches->size() / 2; ++i) {
        for (int j = 0; j < matches->size(); ++j) {
            if (i == j) {
                continue;
            }
            if (abs((*keypoints_fragment)[(*matches)[i].queryIdx].pt.x - (*keypoints_image)[(*matches)[j].queryIdx].pt.x) <  20 &&
                abs((*keypoints_fragment)[(*matches)[i].queryIdx].pt.y - (*keypoints_image)[(*matches)[j].queryIdx].pt.y) < 20 ) {
                return true;
            }
        }
    }
}

// pas reussi a implementer ransac
AngledPosition ransac(std::vector<KeyPoint>* keypoints_image,
                      std::vector<KeyPoint>* keypoints_fragment,
                      std::vector< DMatch>* matches) {
    int n = matches->size();
    AngledPosition result;
    result.none = true;

    for (int i = 0; i < MAX_TRIALS; ++i) {
        int k=0;
        AngledPosition temp;
        temp.x = 0; temp.y = 0; temp.angle = 0; temp.none = false;
        std::vector<DMatch> values = getNRandomValues(MIN_POINTS, matches);
        if (matchesLargerThanImage(&values, keypoints_image, keypoints_fragment)) {
            continue;
        }
        for (auto match : values) {
            temp.x += (*keypoints_fragment)[match.queryIdx].pt.x;
            temp.y += (*keypoints_fragment)[match.queryIdx].pt.y;
            temp.angle += (*keypoints_fragment)[match.queryIdx].angle;
        }
        temp.x /= values.size(); temp.y /= values.size(); temp.angle /= ((float)values.size());
        result = temp;

    }
    return result;
}

void getKeyPoints(Mat image, Mat fragment, std::vector<KeyPoint>* keypoints_image,
        std::vector<KeyPoint>* keypoints_fragment,
                  std::vector< DMatch>* good_matches) {
    //-- Step 1: Detect the keypoints using SURF Detector
    Ptr<FeatureDetector> detector = ORB::create();

    detector->detect(fragment, *keypoints_fragment);
    detector->detect(image, *keypoints_image);

    //-- Step 2: Calculate descriptors (feature vectors)
    Ptr<DescriptorExtractor> extractor = ORB::create();

    Mat descriptors_object, descriptors_scene;

    extractor->compute(fragment, *keypoints_fragment, descriptors_object);
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
        std::cout << "Found match with distance " << match.distance
        << " from " << (*keypoints_image)[match.queryIdx].pt << " to "
        << (*keypoints_fragment)[match.queryIdx].pt
        << " with an angle of " << (*keypoints_fragment)[match.queryIdx].angle << std::endl;
    }
}
int main(int argc, char** argv)
{

    if (argc < 3) {
        std::cout << "You have to provide Miachelangelo and then fragment" << std::endl;
        return -1;
    }
    Mat image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    Mat fragment = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);


    if (!fragment.data || !image.data)
    {
        std::cout << " --(!) Error reading images " << std::endl; return -1;
    }

    std::vector<KeyPoint> keypoints_image;
    std::vector<KeyPoint> keypoints_fragment;
    std::vector< DMatch> good_matches;

    getKeyPoints(image, fragment, &keypoints_image, &keypoints_fragment,
                 &good_matches);
    AngledPosition position = ransac(&keypoints_image, &keypoints_fragment, &good_matches);
    if (position.none) {
        std::cout << "No best point was found" << std::endl;
    } else {
        std::cout << "Found best point (" << position.x << ", " << position.y << ") with angle " << position.angle << std::endl;
    }

    Mat img_matches;

    drawMatches(fragment, keypoints_fragment, image, keypoints_image, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for (int i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints_fragment[good_matches[i].queryIdx].pt);
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
   // imshow("Good Matches & Object detection", img_matches);
    imwrite( "./result.jpg", img_matches );
//    waitKey(0);

    return 0;
}
