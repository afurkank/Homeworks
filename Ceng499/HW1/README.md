# Overall Structure

part3.py script is used for creating the configurations and training a seperate model for each configuration.

find_best.py script is used for defining a function, get_best_config(), that yields the best configuration. It both prints the best configuration and returns a dictionary with the best values.

Here are the hyperparameters I tested:

- number of hidden layers -> [1, 2]
- number of neurons in hidden layers -> [32, 64]
- learning rate -> [0.01, 0.001]
- number of iterations(epoch) ->[400, 500]
- activation functions -> ["sigmoid", "relu"]

There are 32 unique configurations.

Running a script yields a "hyperparameters.txt" file that contains the validation and test accuracy for each configuration.

The columns of the hyperparameters.txt file is like this:
n_hidden,n_neurons,lr,epoch,activation,val_acc,test_acc

Running "get_best_config()" function over this .txt file gets the best individual hyperparameter.

# How to Use

First run "part3.py" script to run experiments on all the configurations.
At the end of this run, a file "hyperparameters.txt" will be created.

To find the best configuration out of all the configurations, you can do:

```
from find_best import get_best_config

result_dict = get_best_config("test_acc", "hyperparameters.txt")

print(result_dict)
```

You can train the best model like this:

```
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
```

You can also run and see the whole process of finding and training with the best configuration in "example_usage.py" script.