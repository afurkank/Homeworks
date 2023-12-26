import numpy as np
from DataLoader import DataLoader

data_path = "data/credit.data"

dataset, labels = DataLoader.load_credit_with_onehot(data_path)
