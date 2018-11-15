#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <fstream>
#include <math.h>
using namespace std;
using namespace cv;

class device
{

public:

    int *exp_time;
    int *hdr_exp_time;
    double a_T[3];
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
        a_T[1] = (1.0/hdr_exp_time[1]) / (1.0/hdr_exp_time[0]);
        a_T[2] = (1.0/hdr_exp_time[2]) / (1.0/hdr_exp_time[0]);
	cout<<"a1 = " << a_T[1]<<", a2 = "<< a_T[2]<<endl;
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
        roi.x = (img[0].cols - cropped_width) / 2;
        roi.y = (img[0].rows - cropped_height) / 2;
        roi.width = cropped_width;
        roi.height = cropped_height;

        for (int i = 0; i < img_num; i++){
            img[i] = img[i](roi);
            cv::imwrite(std::to_string(i) + "_cali.jpg", img[i]);
            //cv::waitKey(10);
        }
        //cv::waitKey(0);

        _checkSaturate(img[img_num - 1]);

        //calculate mean
        //(img_num, b);
        for (int i = 0; i < img_num; i++){
            cv::Scalar mean_tmp = mean( img[i] );
            vector < float > b (mean_tmp.val, mean_tmp.val + 3);
            b_p.push_back(b);
        }


        //linear regression
        _linearRegression(0);
        _linearRegression(1);
        _linearRegression(2);



    }
    void HDR(const string& path){
        img_stack.push_back(cv::imread( path + "1_"+ std::to_string(hdr_exp_time[0] ) +".JPG", CV_LOAD_IMAGE_COLOR));
        img_stack.push_back(cv::imread( path + "1_"+ std::to_string(hdr_exp_time[1] ) +".JPG", CV_LOAD_IMAGE_COLOR));
        img_stack.push_back(cv::imread( path + "1_"+ std::to_string(hdr_exp_time[2] ) +".JPG", CV_LOAD_IMAGE_COLOR));
        for (int i = 0 ; i < 3; i++)
            img_stack[i].convertTo(img_stack[i], CV_32F);
        _radiometriCalibration(img_stack);
        _HDR_method1(img_stack);
        _HDR_method2(img_stack);
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
    void _checkSaturate(const cv::Mat& mat)
    {
        double mat_min, mat_max;
        cv::minMaxLoc(mat, &mat_min, &mat_max);
        if( mat_max >= 255.0)
            std::cout<<"WANRNING: Images have saturated pixels." << endl;
        else
            std::cout<<"All the pixels are non-saturated." << endl;
        std::cout<<"The largest value is "<< mat_max << endl;
    }
    void _HDR_method1(const vector<cv::Mat>& imgs)
    {
        float saturated[] = {std::pow(255, 1.0/b[0]), std::pow(255, 1.0/b[1]),std::pow(255, 1.0/b[2])};
        cv::Mat result = imgs[0].clone();
        for(int j = 0; j < imgs[0].rows; j++)
        {
            for(int i = 0; i < imgs[0].cols; i++)
            {
                for(int c = 0; c < 3; c++)
                {
                    if(imgs[2].at<cv::Vec3f>(j,i)[c] >= saturated[c])
                    {
                        if(imgs[1].at<cv::Vec3f>(j,i)[c] >= saturated[c])
                        {
                            continue;
                        }
                        else
                        {
                            result.at<cv::Vec3f>(j,i)[c] = imgs[1].at<cv::Vec3f>(j,i)[c] / a_T[1];
                        }
                    }
                    else
                        result.at<cv::Vec3f>(j,i)[c] = imgs[2].at<cv::Vec3f>(j,i)[c] / a_T[2];
                }
            }
        }

        _HDRSaveRes(result,imgs[0].rows, imgs[0].cols,"HDR1_res_.txt");
        _ToneMap(result, imgs[0].rows, imgs[0].cols,1);
    }
    void _HDRSaveRes(const cv::Mat& result, int rows, int cols, string path)
    {
        ofstream results_file(path);
        if (results_file.is_open())
        {
            for(int j = 0; j < rows; j++)
            {
                for(int i = 0; i < cols; i++)
                {
                    for(int c = 0; c < 3; c++)
                    {
                        results_file << result.at<cv::Vec3f>(j,i)[c] << " ";
                    }
                    results_file << endl;
                }
            }
            results_file.close();
            std::cout<<"Saved the result of HDR"<<endl;
        }
        else
            std::cout << "Unable to open file";

    }
    void _HDR_method2(const vector<cv::Mat>& imgs)
    {
        float saturated[] = {std::pow(255, 1.0/b[0]), std::pow(255, 1.0/b[1]),std::pow(255, 1.0/b[2])};
        cv::Mat result = imgs[0].clone();
        for(int j = 0; j < imgs[0].rows; j++)
        {
            for(int i = 0; i < imgs[0].cols; i++)
            {
                for(int c = 0; c < 3; c++)
                {
                    if(imgs[2].at<cv::Vec3f>(j,i)[c] >= saturated[c])
                    {
                        if(imgs[1].at<cv::Vec3f>(j,i)[c] >= saturated[c])
                        {
                            continue;
                        }
                        else
                        {
                            result.at<cv::Vec3f>(j,i)[c] = (imgs[1].at<cv::Vec3f>(j,i)[c] / a_T[1] + imgs[0].at<cv::Vec3f>(j,i)[c]) / 2.0;
                        }
                    }
                    else
                        result.at<cv::Vec3f>(j,i)[c] = (imgs[0].at<cv::Vec3f>(j,i)[c] + imgs[1].at<cv::Vec3f>(j,i)[c] / a_T[1] + imgs[2].at<cv::Vec3f>(j,i)[c] / a_T[2] ) / 3.0;
                }
            }
        }

        _HDRSaveRes(result,imgs[0].rows, imgs[0].cols, "HDR2_res_.txt");
        _ToneMap(result, imgs[0].rows, imgs[0].cols, 2);

    }
    void _ToneMap(cv::Mat& img, int rows, int cols, int index)
    {

        cv::Mat res;
        // createTonemapDurand(float gamma=1.0f, float contrast=4.0f, float saturation=1.0f, float sigma_space=2.0f, float sigma_color=2.0f)
        Ptr<TonemapDurand> durand = createTonemapDurand((1.0/b[2] + 1.0/b[1] + 1.0/b[0]) / 3.0, 4.0f, 0.8f, 2.0f, 2.0f);
        durand->process(img, res);

        double mat_min, mat_max, dis;
        cv::minMaxLoc(res, &mat_min, &mat_max);

        for(int j = 0; j < rows; j++)
        {
            for(int i = 0; i < cols; i++)
            {
                for(int c = 0; c < 3; c++)
                {
                    res.at<cv::Vec3f>(j,i)[c] = res.at<cv::Vec3f>(j,i)[c] * 255.0;
                }
            }
        }
        res.convertTo(res, CV_8U );
        cv::imwrite("HDR" + std::to_string(index) + "_res.JPG", res);

    }
    void _calculateHistogram(const cv::Mat& img, string index)
    {
        //calculate Histogram
        MatND hist_r, hist_g, hist_b;
        int imgCount = 1;
        int dims = 25;
        const int sizes[] = {256};
        const int channels_r[] = {0};
        const int channels_g[] = {1};
        const int channels_b[] = {2};
        float Range[] = {0,pow(255, b[2])};
        const float *ranges[] = {Range};
        Mat mask = Mat();
        calcHist(&img, imgCount, channels_r, mask, hist_r, dims, sizes, ranges);
        calcHist(&img, imgCount, channels_g, mask, hist_g, dims, sizes, ranges);
        calcHist(&img, imgCount, channels_b, mask, hist_b, dims, sizes, ranges);
        ofstream results_file("His_" + index + ".txt");

    }
    void _radiometriCalibration(vector<cv::Mat>& imgs)
    {
        for(cv::Mat img : imgs)
        {
            for(int j = 0; j < img.rows; j++)
            {
                for(int i = 0; i < img.cols; i++)
                {
                    Vec3f p = img.at<cv::Vec3f>(j,i);//todo

                    p[0] = std::pow(p[0], 1.0/b[0]);
                    p[1] = std::pow(p[1], 1.0/b[1]);
                    p[2] = std::pow(p[2], 1.0/b[2]);
                    img.at<cv::Vec3f>(j,i) = p;
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
        cout<<a[color_channel]<<" "<<b[color_channel]<<endl;
    }
};

int main()
{

    //int exp_time[] = {350, 250, 180, 125, 90, 60, 45, 30};
    int exp_time[] = {8772, 6410, 4167, 3096, 2037, 1520, 1008, 754, 501, 351};
    int hdr_exp_time[] = {2037, 351, 180};//754, 251, 90 //754,351,180
    int img_num = sizeof(exp_time) / sizeof(*exp_time);
    device device1(exp_time, hdr_exp_time, img_num);
    //device1.radiometriCalibrationParam("src/prt1/1_");
    device1.radiometriCalibrationParam("src/prt1_2/");
    device1.saveDataForPlotting("results.txt");
    device1.HDR("src/prt2/");

    std::cout<<"DONE!!!"<<endl;
    return 0;
}
