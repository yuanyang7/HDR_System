import numpy as np
import matplotlib.pyplot as plt

with open('results.txt') as f:
    lines = f.readlines()
    x = [float(line.split()[0]) for line in lines if len(line.split()) > 3]
    r = [float(line.split()[1]) for line in lines if len(line.split()) > 3]
    g = [float(line.split()[2]) for line in lines if len(line.split()) > 3]
    b = [float(line.split()[3]) for line in lines if len(line.split()) > 3]
    r_c = [float(line.split()[4]) for line in lines if len(line.split()) > 3]
    g_c = [float(line.split()[5]) for line in lines if len(line.split()) > 3]
    b_c = [float(line.split()[6]) for line in lines if len(line.split()) > 3]

    fig = plt.figure(1)

    ax1 = fig.add_subplot(1,1,1)
    #ax1.set_title("Plot title")
    ax1.set_xlabel('T(s)')
    ax1.set_ylabel('B')
    ax1.plot(x, r, c='r', label='r')
    ax1.plot(x, g, c='g', label='g')
    ax1.plot(x, b, c='b', label='b')
    leg = ax1.legend()

    fig2 = plt.figure(2)
    ax2 = fig2.add_subplot(1,1,1)
    #ax1.set_title("Plot title")
    ax2.set_xlabel('T(s)')
    ax2.set_ylabel('B= B\' ^ (1/a)')
    ax2.plot(x, r_c, c='r', label='r')
    ax2.plot(x, g_c, c='g', label='g')
    ax2.plot(x, b_c, c='b', label='b')
    leg = ax2.legend()

    plt.show()
