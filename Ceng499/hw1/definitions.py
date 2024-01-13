import torch.nn as nn
import torch.nn.functional as F
from dataclasses import dataclass

@dataclass
class ModelArgs:
    n_hidden:int
    n_neurons:int
    lr:float
    epoch:int
    activation:str

class classificationModel(nn.Module):
    def __init__(self, args:ModelArgs):
        super(classificationModel, self).__init__()
        self.first_layer = nn.Linear(784, args.n_neurons)
        self.linear_layers = nn.ModuleList(
            [nn.Linear(args.n_neurons, args.n_neurons) for _ in range(args.n_hidden-1)]
        )
        self.output_layer = nn.Linear(args.n_neurons, 10)
        self.activation = args.activation
        self.args = args
    def forward(self, x):
        x = self.first_layer(x)
        if(self.args.n_hidden>1): # skip if there is only one hidden layer
            for layer in self.linear_layers:
                x = layer(x)
                if self.activation == "sigmoid":
                    x = F.sigmoid(x)
                else:
                    x = F.relu(x)
        else:
            if self.activation == "sigmoid":
                x = F.sigmoid(x)
            else:
                x = F.relu(x)
        x = self.output_layer(x)
        return x