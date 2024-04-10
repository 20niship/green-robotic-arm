import matplotlib.pyplot as plt
import numpy as np
import glob


files = glob.glob('dataset/exp1_csv/*.csv')

fname = files[10]

# read csv


def read_csv(fname):
    data = np.loadtxt(fname, delimiter=',', skiprows=1)
    return data

data = read_csv(fname)

plt.plot(data[:, 1], label='x')
plt.plot(data[:, 2], label='y')
plt.plot(data[:, 3], label='z')
plt.plot(data[:, 4], label='a')
plt.plot(data[:, 5], label='b')
plt.plot(data[:, 6], label='c')
plt.plot(data[:, 7], label='d')
plt.plot(data[:, 8], label='e')
plt.legend()
plt.show()
