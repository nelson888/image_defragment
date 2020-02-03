#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <algorithm> 
#include <fstream>
#include <exception>
#include <math.h>

#include <array>
#include<vector>
#include<tuple>
#include<string>
#include<list>
#include<cctype>


using namespace std;
using namespace cv;

int MIN_RADIUS_DETECTION = 5;
int GRADIENT_FILTER_RATIO = 3;
int NUMBER_OF_CIRCLE_TO_DETECT = 10;
int BLUR_SIZE = 1;
float R_RESOLUTION = 1.f;
float C_RESOLUTION = 1.f;
float RAD_RESOLUTION = 1.f;

float RATIO_CASCADE = 2;


Mat load_image(const string& fragment_folder_path);
const string load_file_exception(const string& desc, const string& path);

Mat reduce_noise(const Mat& picture);
Mat get_border(const Mat& picture);
Mat affine_border(const Mat& picture);
list<tuple<int,int,int,int>> matrice_vote(const Mat& picture, const float resolution);
list<tuple<int,int,int,int>> matrice_vote(const Mat& picture, list<tuple<int,int,int,int>> last_votes, float last_res,float res);
list<tuple<int,int,int,int>> filter_local_max(Mat acc);
list<tuple<int,int,int,int>> filter_local_max(Mat acc, list<tuple<int,int,int, int>> last_votes, float delta_rad);
Mat generate_vote_matrix(Mat Picture, vector<Point> borders, float resolution);
vector<Point> get_borders_from_vote(Mat acc,float res);
Mat draw_cirles(Mat background, list<tuple<int,int,int,int>> votes, float votes_resolution);

Mat generate_vote_matrix(Mat picture, vector<Point> borders, list<tuple<int,int,int, int>> last_votes, int delta);

const bool is_inside_acc(const int r, const int c, const int rad, const Mat& acc);
const bool is_local_max(const int r, const int c, const int rad, const Mat& acc);

int main(int argc, char** argv){
    
    //Check arguments
	if (argc < 7 ){
		printf("Usage: <image_path> <gradient filter ratio> <minimum radius detection> <number of circle to detect> <blur size> <cascading_lvl> \n");
		return 1;
	}

	const string image_path(argv[1]);
    GRADIENT_FILTER_RATIO = atoi(argv[2]);
    MIN_RADIUS_DETECTION = atoi(argv[3]);
    NUMBER_OF_CIRCLE_TO_DETECT = atoi(argv[4]);
    BLUR_SIZE = atoi(argv[5]);
    const int CASCADE_LVL = atoi(argv[6]);
	
    Mat initial_picture;
	try {
		initial_picture = load_image(image_path);
	} catch	(string& e) {
		printf("ERROR: %s\n", e.c_str());
		return 2;
	}
	
	// apply filters
	Mat less_noise_picture = reduce_noise(initial_picture);
	Mat border_picture = get_border(less_noise_picture);
    Mat affined_border_picture = affine_border(border_picture);
    
    int res = pow(RATIO_CASCADE,CASCADE_LVL);
    R_RESOLUTION = res;
    C_RESOLUTION = res;
    RAD_RESOLUTION = res;
   	auto locals_maxmimums = matrice_vote(affined_border_picture, res);
    
    Mat tmp = draw_cirles(initial_picture, locals_maxmimums, res);
    imshow( "circle detected picture "+res, tmp);

    while(res > 1){
        res /= RATIO_CASCADE;
        locals_maxmimums = matrice_vote(affined_border_picture, locals_maxmimums, res*2, res);
        
        Mat tmp = draw_cirles(initial_picture, locals_maxmimums, res);
		imshow( "circle detected picture "+res, tmp);
    }    

    
    cout << locals_maxmimums.size() << "\n";
    Mat cirle_detected_picture = draw_cirles(initial_picture, locals_maxmimums, res);
    
	// show result
	if (initial_picture.data && less_noise_picture.data && border_picture.data && affined_border_picture.data ) {
		imshow( "initial picture", initial_picture );
		imshow( "less noise picture", less_noise_picture );
		imshow( "border picture", border_picture );
		imshow( "affined border picture", affined_border_picture);
		imshow( "circle detected picture", cirle_detected_picture);
		waitKey(0);
	}
	return 0;
}

const string load_file_exception(const string& desc, const string& path) {
	return string() + "Fail to load " + desc + ".\n - PATH: [" + path + "]";
}

Mat load_image(const string& fragment_folder_path) {

	const Mat src_img = imread(fragment_folder_path);
	if(! src_img.data ) {
		throw load_file_exception("image", fragment_folder_path );
	}
	return src_img;
}

Mat reduce_noise(const Mat& picture){
	Mat out_picture;
    GaussianBlur( picture, out_picture, Size(BLUR_SIZE,BLUR_SIZE), 0, 0, BORDER_DEFAULT );
    return out_picture;
}

Mat get_border(const Mat& picture){
	    
	Mat src_gray;
    Mat gradient_x, gradient_y;
    Mat magnitude_xy;
    
    /// Convert it to gray
	cvtColor( picture, src_gray, CV_BGR2GRAY );

	/// Gradient X
    Sobel( src_gray, gradient_x, CV_32F, 1, 0);
	
    /// Gradient Y
    Sobel( src_gray, gradient_y, CV_32F, 0, 1);

    /// Magnitude with Gradient X and Gradient Y
    magnitude(gradient_x, gradient_y, magnitude_xy);

    return magnitude_xy;
    
}

Mat affine_border(const Mat& picture) {
    double max;
    Mat affined_border_picture;
    minMaxIdx(picture, nullptr, &max);
	Mat filtered_picture = picture.clone();
    filtered_picture.setTo(0,picture < (max/GRADIENT_FILTER_RATIO));
	return filtered_picture;
}

const bool compare_maximums(const tuple<int,int,int, int>& first, const tuple<int,int,int, int>& second){
	return get<3>(first) > get<3>(second);
}

list<tuple<int,int,int,int>> matrice_vote(const Mat& picture, float res) {

    Mat resized_picture = Mat(Size(picture.cols/res, picture.rows/res), CV_64FC1, Scalar(0));
    resize(picture,resized_picture, resized_picture.size());
    
    vector<Point> borders;
    cv::findNonZero(resized_picture>0, borders);
    Mat acc = generate_vote_matrix(resized_picture, borders,res);
    list<tuple<int,int,int, int>> locals_maxmimums = filter_local_max(acc);
    
    // filter results
	locals_maxmimums.sort(compare_maximums);
	locals_maxmimums.resize(NUMBER_OF_CIRCLE_TO_DETECT);
    
    return locals_maxmimums;

    
}

list<tuple<int,int,int,int>> matrice_vote(const Mat& picture, list<tuple<int,int,int,int>> last_votes, float last_res,float res) {

    Mat resized_picture = Mat(Size(picture.cols/res,picture.rows/res), CV_64FC1, Scalar(0));
    resize(picture,resized_picture, resized_picture.size());
    
    vector<Point> borders;
    cv::findNonZero(resized_picture>0, borders);
    
    int delta = min(8.f,pow(RATIO_CASCADE,(8/res)-1));
    
    Mat acc = generate_vote_matrix(resized_picture, borders,last_votes, delta);
    list<tuple<int,int,int, int>> locals_maxmimums = filter_local_max(acc, last_votes, delta );

    // filter results
	locals_maxmimums.sort(compare_maximums);
	locals_maxmimums.resize(NUMBER_OF_CIRCLE_TO_DETECT);
    
    return locals_maxmimums;

    
}

Mat draw_cirles(Mat background, list<tuple<int,int,int,int>> votes, float res){
    Mat picture = Mat(Size(background.cols/res, background.rows/res), CV_64FC1, Scalar(0));
    resize(background,picture, picture.size());
    
    for(auto circle_data : votes){
        circle(picture,Point(get<0>(circle_data),get<1>(circle_data)),get<2>(circle_data),Scalar(0,0,255));
    }
    
    return picture;
}

Mat generate_vote_matrix(Mat picture, vector<Point> borders, float resolution){
    
    const float res_rad = RAD_RESOLUTION/resolution;
    const float res_r = R_RESOLUTION/resolution;
    const float res_c = C_RESOLUTION/resolution;
    
	const int r_max = picture.rows / res_r;
	const int c_max = picture.cols / res_c;
	const int rad_max = sqrt((picture.rows * picture.rows)+(picture.cols * picture.cols)) / res_rad;
    
    const int sizes[] = {r_max,c_max, rad_max};
    Mat acc(3, sizes, CV_32FC1, cv::Scalar(0));
    
    for(const auto &border_point : borders){   
        
		for(int r = 0; r < r_max; r++){
            for(int c = 0; c < c_max; c++){
                const Point circle_center(r*res_r,c*res_c);
                int rad = norm(circle_center-border_point) / res_rad;
               
                // add possible circle but don't take into account too small circles
                if (rad >= MIN_RADIUS_DETECTION) { 
					acc.at<float>(r, c, rad)+= picture.at<float>(border_point)/rad;
				}
                
            }
        }
    }
    
    return acc;
}

Mat generate_vote_matrix(Mat picture, vector<Point> borders, list<tuple<int,int,int, int>> last_votes, int delta){
        
	const int r_max = picture.rows ;
	const int c_max = picture.cols ;
	const int rad_max = sqrt((picture.rows * picture.rows)+(picture.cols * picture.cols)) ;
    
    const int sizes[] = {r_max,c_max, rad_max};
    Mat acc(3, sizes, CV_32FC1, cv::Scalar(0));
    for(const auto &border_point : borders){   
        // for all points neighbor of last votes
        for( const auto &circle : last_votes){
            int r_min = max(0,(int) (get<0>(circle)*RATIO_CASCADE - delta/2));
            int r_maxi = min(acc.size[0],(int) (get<0>(circle)*RATIO_CASCADE + delta/2));
            int c_min = max(0,(int) (get<1>(circle)*RATIO_CASCADE - delta/2));
            int c_maxi = min(acc.size[1],(int) (get<1>(circle)*RATIO_CASCADE + delta/2));
            for(int r = r_min; r < r_maxi; r++){
                for(int c = c_min; c < c_maxi; c++){
                    const Point circle_center(r,c);
                    int rad = norm(circle_center-border_point);
                
                    // add possible circle but don't take into account too small circles
                    if (rad >= MIN_RADIUS_DETECTION) { 
                        acc.at<float>(r, c, rad)+= picture.at<float>(border_point)/rad;
                    }
                    
                }
            }
        }
    }
    
    return acc;
}

list<tuple<int,int,int, int>> filter_local_max(Mat acc){
    list<tuple<int,int,int, int>> locals_maxmimums;
    for(int r = 0; r < acc.size[0]; r++){
        for(int c = 0; c < acc.size[1]; c++){
            for(int rad = 0; rad < acc.size[2]; rad++){
        		if(is_local_max(r, c, rad, acc)){
                    locals_maxmimums.push_back({r,c,rad, acc.at<float>(r, c, rad)});
                }
            }
        }
    }
    return locals_maxmimums;
}

list<tuple<int,int,int, int>> filter_local_max(Mat acc, list<tuple<int,int,int, int>> last_votes, float delta){
    list<tuple<int,int,int, int>> locals_maxmimums;
    
    for( const auto &circle : last_votes){
        int r_min = max(0,(int) (get<0>(circle)*RATIO_CASCADE - delta/2));
        int r_max = min(acc.size[0],(int) (get<0>(circle)*RATIO_CASCADE + delta/2));
        int c_min = max(0,(int) (get<1>(circle)*RATIO_CASCADE - delta/2));
        int c_max = min(acc.size[1],(int) (get<1>(circle)*RATIO_CASCADE + delta/2));
        int rad_min = max(0,(int) (get<2>(circle)*RATIO_CASCADE - delta/2));
        int rad_max = min(acc.size[2],(int) (get<2>(circle)*RATIO_CASCADE + delta/2));
        for(int r = r_min; r < r_max; r++){
            for(int c = c_min; c < c_max; c++){
                for(int rad = rad_min; rad < rad_max; rad++){
                    if(is_local_max(r, c, rad, acc)){
                        locals_maxmimums.push_back({r,c,rad, acc.at<float>(r, c, rad)});
                    }
                }
            }
        }
    }
    return locals_maxmimums;
}

const bool is_inside_acc(const int r, const int c, const int rad, const Mat& acc){
	return !(r < 0 || c < 0 || rad < 0|| r > acc.size[0]-1 || c > acc.size[1]-1 || rad > acc.size[2]-1);	
}

const bool is_local_max(const int r,const int c,const int rad,const Mat& acc){
	for (int i = -2; i<=2; i++){					
		for (int j = -2; j<=2; j++){
			for (int k= -2; k<=2; k++){
                
                bool is_not_the_checked_point = (i != 0 || j != 0 || k != 0);
                bool is_inside_the_mat = is_inside_acc(r+i, c+j, rad+k, acc);
                bool is_greater_than_the_checked_point = is_inside_the_mat && acc.at<int>(r+i, c+j, rad+k) >= acc.at<int>(r, c, rad);
                
                if(is_not_the_checked_point && is_greater_than_the_checked_point) {
                    return false; 
                }
			}
		}
	}
	return true;
}












