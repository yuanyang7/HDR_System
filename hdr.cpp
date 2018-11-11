#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <math.h>
using namespace std;
using namespace cv;

class device
{

public:

    int *exp_time;
    int img_num;
    vector<cv::Mat> img;
    vector < vector < float > > b_p;

    //linearRegression
    vector <float> a;
    vector <float> b;

    //regions for cropping
    int cropped_width;
    int cropped_height;

    device(int *exp_time, int img_num) : a(3, 0.0), b(3, 0.0)
    {
        this->exp_time = exp_time;
        this->img_num = img_num;
        cropped_width = 768;
        cropped_height = 512;
    }
    void radiometriCalibration(string path)
    {
        //read images from folder
        for(int i = 0; i < img_num; i++){
            cv::Mat img_tmp = cv::imread( path + std::to_string(exp_time[i]) + ".jpeg", CV_LOAD_IMAGE_COLOR);
            if(! img_tmp.data )                              // Check for invalid input
            {
                cout <<  "Error: Could not open or find the " + std::to_string(i) +"th image" << std::endl ;
            }
            else
                img.push_back(img_tmp);
        }

        //cropping
        cv::Rect roi;
        roi.x = (img[0].cols - cropped_height) / 2;
        roi.y = (img[0].rows - cropped_width) / 2;
        roi.width = cropped_width;
        roi.height = cropped_height;

        for (int i = 0; i < img_num; i++){
            img[i] = img[i](roi);
            //cv::imshow("crop", img[i]);
            //cv::waitKey(0);
        }

        //if saturate
        double mat_min, mat_max;
        cv::minMaxLoc(img[img_num - 1], &mat_min, &mat_max);
        if( mat_max == 255.0)
            std::cout<<"WANRNING: Images have saturated pixels." << endl;
        else
            std::cout<<"All the pixels are non-saturated." << endl;

        //calculate mean
        //(img_num, b);
        for (int i = 0; i < img_num; i++){
            cv::Scalar mean_tmp = mean( img[i] );
            vector < float > b (mean_tmp.val,mean_tmp.val + 3);
            b_p.push_back(b);
        }


        //linear regression
        _linearRegression(0); //r
        _linearRegression(1); //g
        _linearRegression(2); //b



    }

    void saveDataForPlotting(string path)
    {
        ofstream results_file(path);
        if (results_file.is_open())
        {
            for(int i = 0; i < img_num; i++){
                results_file << 1.0 / exp_time[i] << " ";
                results_file << b_p[i][0] << " " ;
                results_file << b_p[i][1] << " " ;
                results_file << b_p[i][2] << " " ;
                results_file << std::pow(b_p[i][0], (1.0 / b[0])) << " ";
                results_file << std::pow(b_p[i][1], (1.0 / b[1])) << " ";
                results_file << std::pow(b_p[i][2], (1.0 / b[2])) << " ";

                results_file<<endl;
            }

            results_file << a[0] << " " << b[0] << endl;
            results_file << a[1] << " " << b[1] << endl;
            results_file << a[2] << " " << b[2] << endl;
            results_file.close();
            std::cout << "Saved the results..." << endl;
        }
        else
            std::cout << "Unable to open file";
    }
private:
    void _linearRegression(int color_channel)
    {
        float mean_y = 0;
        float mean_x = 0;
        for (int i = 0; i < img_num; i++)
        {
            mean_y += log(b_p[i][color_channel]);
            mean_x += log(1.0 / exp_time[i]);
        }

        mean_x /= img_num;
        mean_y /= img_num;

        //b
        float sum_t1 = 0, sum_t2 = 0;
        for(int i = 0; i < img_num; i++){
            sum_t1 += (log(1.0 / exp_time[i]) - mean_x) * (log(b_p[i][color_channel]) - mean_y);
            sum_t2 += (log(1.0 / exp_time[i]) - mean_x) * (log(1.0 / exp_time[i]) - mean_x);
        }
        b[color_channel] = sum_t1 / sum_t2;
        a[color_channel] = mean_y - b[color_channel] * mean_x;
        cout<<a[color_channel]<<" "<<b[color_channel]<<endl;
    }
};

int main()
{
    int exp_time[] = {350, 250, 180, 125, 90, 60, 45, 30};
    int img_num = sizeof(exp_time) / sizeof(*exp_time);
    device device1(exp_time, img_num);
    device1.radiometriCalibration("src/prt1/1_");
    device1.saveDataForPlotting("results.txt");


    std::cout<<"DONE!!!"<<endl;
    return 0;
}
