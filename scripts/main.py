import os
import sys
import argparse
import numpy as np
import scipy.sparse
import ipdb

sys.path.append('scripts')

from benchmark import profile_results

def load_results(path:str) -> (np.array, np.array):
    '''
    '''
    J = np.load(f"{path}/J_star_alpha_0_99.npy")
    Pi = np.load(f"{path}/pi_star_alpha_0_99.npy")

    return (J, Pi)

def load_implementations(dataset:str) -> (dict):
    implementations = {}
    files = []
    for f in os.listdir(f"results/{dataset}"):
        if f.endswith(".npz"):
            files.append(f)

    if len(files) == 0:
        return implementations

    for f in files:
        data = np.load(f"results/{dataset}/{f}")
        name = (f.split("/")[-1]).split(".")[0]
        scheme = name.split("_")[0]
        scheme = scheme[2:]

        implementations[name] = {}
        implementations[name]["dataset"] = dataset
        implementations[name]["name"] = name
        implementations[name]["scheme"] = scheme
        implementations[name]["num_runs"] = data["num_runs"].astype(int)[0]
        implementations[name]["num_procs"] = data["num_procs"].astype(int)[0]
        implementations[name]["frequency"] = data["frequency"].astype(int)[0]
        implementations[name]["execution_times"] = data["execution_times"].astype(float)
        implementations[name]["comm_times"] = data["comm_times"].astype(float)
        implementations[name]["trajectory"] = data["trajectory"].astype(float)
        implementations[name]["policy"] = data["policy"].astype(int)
        implementations[name]["nr_states"] = data["nr_states"].astype(int)
        implementations[name]["nr_actions"] = data["nr_actions"].astype(int)

        execution_times = data["execution_times"].astype(float)
        nr_states = data["nr_states"].astype(int)[0]
        nr_actions = data["nr_actions"].astype(int)[0]
        p_complexity = nr_actions *nr_states
        weighted_mean_execution_times = [time / p_complexity for time in execution_times]

        implementations[name]["weighted_mean_execution_times"] = weighted_mean_execution_times

    return implementations

def benchmark():
    results = {}
    for dataset in ("debug", "small", "normal"):
        implementation = load_implementations(f"{dataset}")
        results[dataset] = implementation

    profile_results(results)
        
def main():
    benchmark()


if __name__ == "__main__":
    main()
