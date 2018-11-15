# HDR_System
## environment
*  OS: Mac OS 10.13.3
*  Language: C++11, Python3
*  Libraries: **For C++** : OpenCV3.4. **For Python3**: OpenCV, numpy, matplotlib

## How to Run
*  **Run cpp file for calibration, calculate HDR and save HDR results:**  
 If you have **c++ and opencv** environment, you can use your IDE to run the code **"hdr.cpp"** directly.  

 Or if you are a Mac user: In terminal, go to the root directory of this project, **run command "make"**.  

 The results images will be saved at the project root directory as "HDR1_res.JPG" and "HDR2_res.JPG". The folder "res" contains all the plotting and image results.

*  **Plot:**  
 **After running "hdr.cpp"**, the data used for plotting will be saved in the folder.  
 **run command "python3 plot.py"** to plot the figures for question 1.  
 **run command "python3 plot_histogram.py -q2"** to plot the histograms for question 2.  
 **run command "python3 plot_histogram.py -q3"** to plot the histograms for question 3 (Should run hdr.cpp first).  
