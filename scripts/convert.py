import os
import pickle
import numpy as np

def convert():
    folder = os.path.join(os.getcwd(), "data")
    for name in os.listdir(folder):
        subfolder = os.path.join(folder, name)
        if os.path.isdir(subfolder):
            with open(os.path.join(subfolder,"parameters.pickle"), "rb") as the_file:
                parameters = pickle.load(the_file)
                
            print(f"Converting file {subfolder}/parameters.pickle -> {subfolder}/parameters.npz")
            np.savez(os.path.join(subfolder,"parameters.npz"), **parameters)

if __name__ == "__main__":
    convert()
