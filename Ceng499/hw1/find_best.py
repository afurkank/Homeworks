import pandas as pd

# function to find the best value for each hyperparameter
def find_best_hyperparameters(df, hyperparameters, acc_type='val_acc'):
    best_values = {}
    
    for param in hyperparameters:
        # group by the hyperparameter and calculate the mean of the specified accuracy
        mean_accs = df.groupby(param)[acc_type].mean()
        
        # find the value of the hyperparameter that has the highest mean accuracy
        best_value = mean_accs.idxmax()
        
        # store the best value
        best_values[param] = best_value

    return best_values
def get_best_config(acc_type, txt_file_name):
    # load the data
    df = pd.read_csv(txt_file_name)

    # list of hyperparameters
    hyperparameters = ['n_hidden', 'n_neurons', 'lr', 'epoch', 'activation']

    # find the best values for each hyperparameter
    best_hyperparameter_values = find_best_hyperparameters(df, hyperparameters, acc_type)

    # print the best values for each hyperparameter
    result_dict = {}
    print("Best individual hyperparameter values:")
    for param, value in best_hyperparameter_values.items():
        result_dict[f"{param}"] = value
        print(f"{param}: {value}")

    return result_dict