import numpy as np

class Distance:
    @staticmethod
    def calculateCosineDistance(x, y, param=None):
        numerator = np.dot(x,y)
        denominator = np.linalg.norm(x) * np.linalg.norm(y)
        return 1 - (numerator / denominator)
    @staticmethod
    def calculateMinkowskiDistance(x, y, p=2):
        return np.sum(np.abs(x - y) ** p) ** (1 / p)
    @staticmethod
    def calculateMahalanobisDistance(x,y, S_minus_1):
        return np.sqrt(np.dot(np.dot((x - y).T, S_minus_1), x - y))