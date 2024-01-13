import pickle
import numpy as np
from sklearn.pipeline import make_pipeline
from sklearn.svm import SVC
import matplotlib.pyplot as plt

dataset, labels = pickle.load(open("../data/part2_dataset1.data", "rb"))

# hyperparameters to test
Cs = [0.1, 10000]
kernels = ['linear', 'poly']
degree = 10

# function to create a mesh grid for plotting decision boundaries
def make_meshgrid(x, y, h=.02):
    x_min, x_max = x.min() - 1, x.max() + 1  # set the min and max values for x-axis
    y_min, y_max = y.min() - 1, y.max() + 1  # set the min and max values for y-axis
    xx, yy = np.meshgrid(np.arange(x_min, x_max, h),
                         np.arange(y_min, y_max, h))  # create meshgrid for plotting
    return xx, yy

# function to plot the decision contours
def plot_contours(ax, clf, xx, yy, **params):
    Z = clf.predict(np.c_[xx.ravel(), yy.ravel()])  # make predictions over the grid
    Z = Z.reshape(xx.shape)  # reshape the predictions to match xx's dimensions
    out = ax.contourf(xx, yy, Z, **params)  # plot the decision contours
    return out

X0, X1 = dataset[:, 0], dataset[:, 1]  # split the dataset into two features for plotting
xx, yy = make_meshgrid(X0, X1)  # create a mesh grid for plotting

# loop over combinations of hyperparameters
for c in Cs:
    for kernel in kernels:
        # create and fit the SVM model with specific hyperparameters
        clf = make_pipeline(SVC(C=c, kernel=kernel, gamma='auto', degree=degree))
        clf.fit(dataset, labels)  # train the model with the dataset

        plt.figure(figsize=(8, 6))  # create a new figure for plotting
        ax = plt.gca()  # get the current axes for plotting
        plot_contours(ax, clf, xx, yy, cmap=plt.cm.coolwarm, alpha=0.8)  # plot the decision boundaries
        ax.scatter(X0, X1, c=labels, cmap=plt.cm.coolwarm, s=20, edgecolors='k')  # plot the dataset points
        ax.set_xlim(xx.min(), xx.max())  # set the limits for x-axis
        ax.set_ylim(yy.min(), yy.max())  # set the limits for y-axis
        ax.set_xlabel('x1')  # label for x-axis
        ax.set_ylabel('x2')  # label for y-axis

        # set the title of the plot based on the kernel type
        if kernel == 'linear':
            ax.set_title(f'SVM with C={c}, kernel={kernel}')
        else:
            ax.set_title(f'SVM with C={c}, kernel={kernel}-degree={degree}')

        plt.savefig(f"C{c}_kernel{kernel}.png")  # save the plot to a file
        plt.show()  # display the plot
