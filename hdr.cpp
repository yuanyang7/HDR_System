#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

int main()
{
    //exposure time
    int exp_time[] = {30, 45, 60, 90, 125, 180, 250, 350};
    int img_num = sizeof(exp_time) / sizeof(*exp_time);

    //read images from folder
    vector<cv::Mat> img;
    for(int i = 0; i < img_num; i++){
        cv::Mat img_tmp = cv::imread("src/prt1/1_" + std::to_string(exp_time[i]) + ".jpg", CV_LOAD_IMAGE_COLOR);
        if(! img_tmp.data )                              // Check for invalid input
        {
            cout <<  "Could not open or find the " + std::to_string(i) +"th image" << std::endl ;
            return -1;
        }
        else
            img.push_back(img_tmp);
    }

    //set the region for cropping
    int cropped_width = 768;
    int cropped_height = 512;

    cv::Rect roi;
    roi.x = (img[0].cols - cropped_height) / 2;
    roi.y = (img[0].rows - cropped_width) / 2;
    roi.width = cropped_width;
    roi.height = cropped_height;

    //crop
    for (int i = 0; i < img_num; i++){
        img[i] = img[i](roi);
        cv::imshow("crop", img[i]);
        cv::waitKey(0);
    }


    cout<<"DONE.."<<endl;
                                         // Wait for a keystroke in the window
    return 0;
}
