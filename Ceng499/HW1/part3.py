import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
import pickle
from definitions import ModelArgs, classificationModel

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

# we load all the datasets of Part 3
x_train, y_train = pickle.load(open("data/mnist_train.data", "rb"))
x_validation, y_validation = pickle.load(open("data/mnist_validation.data", "rb"))
x_test, y_test = pickle.load(open("data/mnist_test.data", "rb"))

x_train = x_train/255.0
x_train = x_train.astype(np.float32)

x_test = x_test / 255.0
x_test = x_test.astype(np.float32)

x_validation = x_validation/255.0
x_validation = x_validation.astype(np.float32)

# and converting them into Pytorch tensors in order to be able to work with Pytorch
x_train = torch.from_numpy(x_train).to(device)
y_train = torch.from_numpy(y_train).to(torch.long).to(device)

x_validation = torch.from_numpy(x_validation).to(device)
y_validation = torch.from_numpy(y_validation).to(torch.long).to(device)

x_test = torch.from_numpy(x_test).to(device)
y_test = torch.from_numpy(y_test).to(torch.long).to(device)

# hyperparemeters
"""

# of hidden layers
# of neurons in hidden layers
# learning rate
# number of iterations(epoch)
# activation functions

"""

num_hidden_layers = [1, 2]
num_neurons = [32, 64]
learning_rates = [0.01, 0.001]
num_epochs = [20, 30]
activation_funcs = ["sigmoid", "relu"]

model_configs = []

for n_hidden in num_hidden_layers:
    for n_neuron in num_neurons:
        for lr in learning_rates:
            for epoch in num_epochs:
                for activation in activation_funcs:
                    config = {
                        "n_hidden": n_hidden,
                        "n_neurons": n_neuron,
                        "lr": lr,
                        "epoch": epoch,
                        "activation": activation
                    }
                    model_configs.append(config)

test_acc_arr = []
val_acc_arr = []

print(f"There are {len(model_configs)} model configurations\n")

# run experiment for each model configuration
for i,model_config in enumerate(model_configs):
    validation_accs = []
    test_accs = []

    # run experiment 10 times and get average of both validation and test accuracy
    for _ in range(1):
        # reset model
        model = classificationModel(ModelArgs(**model_config)).to(device)
        # optimizer
        optimizer = optim.SGD(model.parameters(), lr=model.args.lr)
        # loss function
        loss_func = nn.CrossEntropyLoss()

        val_acc = []

        # train
        for epoch in range(model.args.epoch):
            optimizer.zero_grad()
            y_pred = model(x_train)
            loss = loss_func(y_pred, y_train)
            loss.backward()
            optimizer.step()

            # validation
            with torch.no_grad():
                y_pred = model(x_validation)
                validation_accuracy = (torch.sum(y_pred.argmax(dim=-1) == y_validation) / y_validation.shape[0]) * 100
                val_acc.append(validation_accuracy)
        
        validation_accs.append(np.mean(val_acc))
        
        # test
        with torch.no_grad():
            y_pred = model(x_test)
            test_accuracy = (torch.sum(y_pred.argmax(dim=-1) == y_test) / y_test.shape[0]) * 100
            test_accs.append(test_accuracy)
    
    val_acc_arr.append(np.mean(validation_accs))
    test_acc_arr.append(np.mean(test_accs))
    
    print(f"Config {i+1}/{len(model_configs)}:\n# hidden layers={model_config['n_hidden']}, # neurons={model_config['n_neurons']}, lr={model_config['lr']}, epoch={model_config['epoch']}, activation={model_config['activation']}")
    
    print(f"validation accuracy: {np.mean(validation_accs):.3f}")
    
    print(f"test accuracy: {np.mean(test_accs):.3f}")

# write validation and test accuracy to txt file
# along with model configs
with open("hyperparameters.txt", "w") as f:
    f.write("n_hidden,n_neurons,lr,epoch,activation,val_acc,test_acc\n")
    for i, model_config in enumerate(model_configs):
        f.write(f"{model_config['n_hidden']},{model_config['n_neurons']},{model_config['lr']},{model_config['epoch']},{model_config['activation']},{val_acc_arr[i]},{test_acc_arr[i]}\n")