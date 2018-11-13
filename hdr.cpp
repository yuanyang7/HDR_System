#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <math.h>
using namespace std;
using namespace cv;

class device
{

public:

    int *exp_time;
    int *hdr_exp_time;
    int img_num;
    vector<cv::Mat> img;
    vector<cv::Mat> img_stack;
    vector < vector < float > > b_p;

    //linearRegression
    vector <float> a;
    vector <float> b;

    //regions for cropping
    int cropped_width;
    int cropped_height;

    device(int *exp_time, int *hdr_exp_time, int img_num) : a(3, 0.0), b(3, 0.0)
    {
        this->exp_time = exp_time;
        this->hdr_exp_time = hdr_exp_time;
        this->img_num = img_num;
        cropped_width = 768;
        cropped_height = 512;
    }
    void radiometriCalibrationParam(const string& path)
    {
        //read images from folder
        for(int i = 0; i < img_num; i++){
            cv::Mat img_tmp = cv::imread( path + std::to_string(exp_time[i]) + ".JPG", CV_LOAD_IMAGE_COLOR);
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

        checkSaturate(img[img_num - 1]);

        //calculate mean
        //(img_num, b);
        for (int i = 0; i < img_num; i++){
            cv::Scalar mean_tmp = mean( img[i] );
            vector < float > b (mean_tmp.val, mean_tmp.val + 3);
            b_p.push_back(b);
        }


        //linear regression
        _linearRegression(0); //r
        _linearRegression(1); //g
        _linearRegression(2); //b



    }
    void HDR(const string& path){
        //190, 208
        //select: 251, 60, 2037(or 754)
        img_stack.push_back(cv::imread( path + "1_2037.JPG", CV_LOAD_IMAGE_COLOR));
        img_stack.push_back(cv::imread( path + "1_251.JPG", CV_LOAD_IMAGE_COLOR));
        img_stack.push_back(cv::imread( path + "1_60.JPG", CV_LOAD_IMAGE_COLOR));

        _radiometriCalibration(img_stack);
        //_calculateHistogram(img_stack[0],hdr_exp_time[0]);
        //_calculateHistogram(img_stack[1],hdr_exp_time[1]);
        //_calculateHistogram(img_stack[2],hdr_exp_time[2]);
        _saveImageInfo(img_stack[0],hdr_exp_time[0]);
        _saveImageInfo(img_stack[1],hdr_exp_time[1]);
        _saveImageInfo(img_stack[2],hdr_exp_time[2]);




    }
    void checkSaturate(const cv::Mat& mat)
    {
        double mat_min, mat_max;
        cv::minMaxLoc(mat, &mat_min, &mat_max);
        if( mat_max == 255.0)
            std::cout<<"WANRNING: Images have saturated pixels." << endl;
        else
            std::cout<<"All the pixels are non-saturated." << endl;
        std::cout<<"The largest value is "<< mat_max << endl;
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
    void _saveImageInfo(const cv::Mat& img, int index)
    {
        /*
        ofstream results_file("Img_" + std::to_string(index) + ".txt");
        if (results_file.is_open())
        {
            for(int j = 0; j < img.rows; j++)
            {
                for(int i = 0; i < img.cols; i++)
                {
                    results_file << img.at<Vec3b>(Point(i, j)).val[0] <<" " << img.at<Vec3b>(Point(i, j)).val[1] <<" " << img.at<Vec3b>(Point(i, j)).val[2];
                    results_file<<endl;
                }
            }
            results_file.close();
            std::cout << "Saved the histogram results of " +  std::to_string(index) << endl;
        }
        else
            std::cout << "Unable to open file";
        */
        cv::imwrite("Img_" + std::to_string(index) + ".jpg", img);
    }
    void _calculateHistogram(const cv::Mat& img, int index)
    {
        //calculate Histogram
        MatND hist_r, hist_g, hist_b;
        int imgCount = 1;
        int dims = 1;
        const int sizes[] = {256};
        const int channels_r[] = {0};
        const int channels_g[] = {1};
        const int channels_b[] = {2};
        float Range[] = {0,256};
        const float *ranges[] = {Range};
        Mat mask = Mat();
        calcHist(&img, imgCount, channels_r, mask, hist_r, dims, sizes, ranges);
        calcHist(&img, imgCount, channels_g, mask, hist_g, dims, sizes, ranges);
        calcHist(&img, imgCount, channels_b, mask, hist_b, dims, sizes, ranges);
        ofstream results_file("His_" + std::to_string(index) + ".txt");
        if (results_file.is_open())
        {
            for(int i = 0; i < 256; i++){
                results_file << *hist_r.ptr<float>(i) <<" " << *hist_g.ptr<float>(i) <<" " << *hist_b.ptr<float>(i);
                results_file<<endl;
            }
            results_file.close();
            std::cout << "Saved the histogram results of " +  std::to_string(index) << endl;
        }
        else
            std::cout << "Unable to open file";

    }
    void _radiometriCalibration(vector<cv::Mat>& imgs)
    {
        for(cv::Mat img : imgs)
        {
            for(int j = 0; j < img.rows; j++)
            {
                for(int i = 0; i < img.cols; i++)
                {
                    img.at<Vec3b>(Point(i, j)).val[0] = std::pow(img.at<Vec3b>(Point(i, j)).val[0], 1.0/b[0]);
                    img.at<Vec3b>(Point(i, j)).val[1] = std::pow(img.at<Vec3b>(Point(i, j)).val[1], 1.0/b[1]);
                    img.at<Vec3b>(Point(i, j)).val[2] = std::pow(img.at<Vec3b>(Point(i, j)).val[2], 1.0/b[2]);
                }
            }
        }

    }
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
        //cout<<a[color_channel]<<" "<<b[color_channel]<<endl;
    }
};

int main()
{

    //int exp_time[] = {350, 250, 180, 125, 90, 60, 45, 30};
    int exp_time[] = {8772, 6410, 4167, 3096, 2037, 1520, 1008, 754, 501, 351};
    int hdr_exp_time[] = {2037, 251, 60};
    int img_num = sizeof(exp_time) / sizeof(*exp_time);
    device device1(exp_time, hdr_exp_time, img_num);
    //device1.radiometriCalibrationParam("src/prt1/1_");
    device1.radiometriCalibrationParam("src/prt1_2/");
    device1.saveDataForPlotting("results.txt");
    device1.HDR("src/prt2/");

    std::cout<<"DONE!!!"<<endl;
    return 0;
}
