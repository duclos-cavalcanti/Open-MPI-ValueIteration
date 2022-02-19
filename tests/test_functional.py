import os
import sys
import pytest
import pytest_check as check
from tabulate import tabulate
from termcolor import colored

sys.path.append('scripts')

from main import load_implementations, load_results

class Fault:
    def __init__(self, wrong, correct, i):
        self.values = (wrong, correct)
        self.position = i


def print_faults(faults, header):
    table = []
    for f in faults:
        wrong = "{:.2f}".format(round(f.values[0], 2))
        correct = "{:.2f}".format(round(f.values[1], 2))
        i = f.position

        entry = [ colored(f"Index: {i}", "white", None), 
                  colored(f"Wrong: {wrong}", "red", None), 
                  colored(f"Correct: {correct}", "green", None)
                ]

        table.append(entry)


    content = colored(f"\n{header}\n", "blue", None)
    content += f"{tabulate(table)}"
    return content

def evaluatePolicy(Pi, Pi_Star):
    faults = []
    i = 0
    for pi, pi_star in zip(Pi, Pi_Star):
        if pi != pi_star:
            f = Fault(pi, pi_star, i)
            faults.append(f)

        i += 1

    return faults

def evaluateTrajectory(J, J_Star):
    faults = []
    i = 0
    for j, j_star in zip(J, J_Star):
        if j_star == 0.0:
            if j != j_star:
                f = Fault(round(j, 2), round(j_star, 2), i)
                faults.append(f)

        else:
            diff = abs(round(j_star, 2)) - abs(round(j, 2))
            perc = abs(diff / abs(j_star)) * 100
            if perc > 0.1:
                f = Fault(round(j, 2), round(j_star, 2), i)
                faults.append(f)
        i += 1

    return faults

def test_value_iteration():
    for dataset in ("debug", "small", "normal"):
        results = load_implementations(f"{dataset}")

        if len(results) == 0:
            continue

        for impl in results.keys():
            J_Star, Pi_Star = load_results(f"data/data_{dataset}")
            for i in range(results[impl]['num_runs']):
                J = results[impl]["trajectory"][i]
                Pi = results[impl]["policy"][i]

                faults = evaluateTrajectory(J, J_Star)
                header = f"J: [{impl}, {dataset}, {i}, {len(faults)}]"
                check.equal(len(faults), 0, f"{print_faults(faults, header)}")

                faults = evaluatePolicy(Pi, Pi_Star)
                header = f"Pi: [{impl}, {dataset}, {i}, {len(faults)}]"
                check.equal(len(faults), 0, f"{print_faults(faults, header)}")

                print(f"Passed {impl}")

# if __name__ == "__main__":
#     test_value_iteration()
