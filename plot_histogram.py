import numpy as np
import matplotlib.pyplot as plt
import cv2
from matplotlib.ticker import MultipleLocator, FormatStrFormatter, LogFormatterSciNotation, MaxNLocator

def plot(img1, index, div = False):
    
    if(div == True):
        img1[:,0] = img1[:,0] / a[index - 2]
        img1[:,1] = img1[:,1] / a[index - 2]
        img1[:,2] = img1[:,2] / a[index - 2]
    else:
        img1[:,0] = pow(img1[:,0], 1 / b[0])
        img1[:,1] = pow(img1[:,1], 1 / b[1])
        img1[:,2] = pow(img1[:,2], 1 / b[2])	

    if (div == False):
        fig = plt.figure(index)
        plt.suptitle("Histogram B’g(a_"+ str(index - 1) +"*T)")
    else:
        fig = plt.figure(index + 2)
        plt.suptitle("Histogram B’g(a_"+ str(index - 1) +"*T)/a_" + str(index - 1))
	
    ax1 = fig.add_subplot(1,3,1)
    #ax1.set_xlabel('T(s)')
    #ax1.set_ylabel('B')
    ax1.set_title("R")
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))  
    #ax1.yaxis.set_major_locator(MaxNLocator(integer=True))
    #ax1.xaxis.set_major_formatter(LogFormatterSciNotation(base = 1/b[0]))
    #xlabs = [int(pow(i,1/b[0])) for i in range(0,255, 80)]
    #ax1.set_xticklabels(xlabs)
    #ax1.set_xticks(xlabs)

    ax1.hist(img1[:,0], range = (0, pow(255, 1 / b[0])), bins=n_bins)

    ax2 = fig.add_subplot(1,3,2)
    #ax2.set_xlabel('T(s)')
    #ax2.set_ylabel('B')
    ax2.set_title("G")
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))  
    ax2.hist(img1[:,1], range = (0, pow(255, 1 / b[1])), bins=n_bins)

    ax3 = fig.add_subplot(1,3,3)
    #ax3.set_xlabel('T(s)')
    #ax3.set_ylabel('B')
    ax3.set_title("B")
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0,0))  
    ax3.hist(img1[:,2], range = (0, pow(255, 1 / b[2])), bins=n_bins)



with open('results.txt') as f:
    lines = f.readlines()
    a = [float(line.split()[0]) for line in lines if len(line.split()) < 3]
    b = [float(line.split()[1]) for line in lines if len(line.split()) < 3]
n_bins = 25
img = [np.asarray(cv2.imread('src/prt2/1_754.JPG'), dtype=np.uint32),
       np.asarray(cv2.imread('src/prt2/1_251.JPG'), dtype=np.uint32),
       np.asarray(cv2.imread('src/prt2/1_90.JPG'), dtype=np.uint32)]
t0 = 1.0/754
t1 = 1.0/251
t2 = 1.0/90
a[0] = t1 * 1.0 / t0
a[1] = t2 * 1.0 / t0
img[0] = img[0].reshape(-1, img[0].shape[-1])
img[1] = img[1].reshape(-1, img[1].shape[-1])
img[2] = img[2].reshape(-1, img[2].shape[-1])

plot(img[0], 1)
plot(img[1], 2)
plot(img[2], 3)
plot(img[1], 2, True)
plot(img[2], 3, True)

plt.show()
