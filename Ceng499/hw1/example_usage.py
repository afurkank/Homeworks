from definitions import ModelArgs
from find_best import get_best_config
from train_best_config import train_model

result_dict = get_best_config("test_acc", "hyperparameters.txt")

print(result_dict)

args: ModelArgs = ModelArgs(
    n_hidden=result_dict["n_hidden"],
    n_neurons=result_dict["n_neurons"],
    lr=result_dict["lr"],
    epoch=result_dict["epoch"],
    activation=result_dict["activation"]
)

train_model(args)