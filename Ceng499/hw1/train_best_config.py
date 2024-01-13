import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
import pickle
import matplotlib.pyplot as plt

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

def train_model(args:ModelArgs):
    model = classificationModel(args)

    optimizer = optim.SGD(model.parameters(), lr=model.args.lr)

    loss_func = nn.CrossEntropyLoss()

    iters = []
    train_losses = []
    validation_losses = []

    # train
    y_pred = model(x_validation)
    initial_val_acc = (torch.sum(y_pred.argmax(dim=-1) == y_validation) / y_validation.shape[0]) * 100
    y_pred = model(x_test)
    initial_test_acc = (torch.sum(y_pred.argmax(dim=-1) == y_test) / y_test.shape[0]) * 100
    
    print("\nTraining starts\n")
    train_loss = None
    val_loss = None
    for i in range(1000):
        iters.append(i)
        optimizer.zero_grad()
        y_pred = model(x_train)
        loss = loss_func(y_pred, y_train)
        loss.backward()
        optimizer.step()
        train_loss = loss.item()
        train_losses.append(train_loss)
        # validation
        with torch.no_grad():
            y_pred = model(x_validation)
            val_loss = loss_func(y_pred, y_validation).item()
            validation_losses.append(val_loss)
        print(f"Epoch {i+1}/1000 train loss: {train_loss:.3f}, validation loss: {val_loss:3f}")
    print("\nTraining is complete\n")
    print(f"\nValidation and test accuracies before training:\nValidation accuracy:{initial_val_acc}, Test accuracy: {initial_test_acc}\n")
    with torch.no_grad():
        y_pred = model(x_validation)
        validation_accuracy = (torch.sum(y_pred.argmax(dim=-1) == y_validation) / y_validation.shape[0]) * 100
        print(f"validation accuracy: {validation_accuracy:.3f}")
        y_pred = model(x_test)
        test_accuracy = (torch.sum(y_pred.argmax(dim=-1) == y_test) / y_test.shape[0]) * 100
        print(f"test accuracy: {test_accuracy:.3f}")
    # plot loss graph
    plt.plot(iters, train_losses, color='green', label='Traninig loss')
    plt.plot(iters, validation_losses, color='blue', label='Validation loss')
    plt.title("Loss Graph")
    plt.xlabel("Epoch")
    plt.ylabel("Loss")

    # Adding the legend
    plt.legend()

    plt.savefig('loss_graph.png')

    plt.show()