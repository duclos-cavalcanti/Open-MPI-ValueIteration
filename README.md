# Gruppe 5: Value Iteration with MPI

## Introduction
This was the product of a group project at the Technical University of Munich. The goal was implement a asynchronous
value iteration model using OpenMPI to distribute the workload across different machines. 

Many thanks to the lecturers that oversaw the work and provided such an amazing opportunity:

|      Lecturer               |         Email               |
| --------------------------- | --------------------------- |
| Gottwald, Martin (PhD)      | martin.gottwald@tum.de      |
| Hein, Alice (PhD)           | alice.hein@tum.de           |

## Dependencies
### Toolchain
1. `CMake`
2. `OpenMPI`

#### Debian-based Systems
```sh
TODO
```

#### Arch-based Systems
```sh
sudo pacman -S cmake openmpi
```
### Development
#### Python Libraries
```sh
pip install numpy scipy termcolor tabulate pytest pytest-check pandas matplotlib seaborn
```

## Getting Started
### Targets
1. `make project`:
This is the only target you actually need to run if on the HPC server. It will clean the
project, convert pickle files, compile the code and start the execution of the project
using the `debug` and `small` datasets with 2,4 and 6 processes.

2. `make run`:
Simple target to run on the HPC server to debug/run the implementation
quickly with only 2 and 4 processes. Only runs the debug dataset by default.

3. `make local`:
Simple target to run on locally to debug/run the implementation
quickly with only 2 processes. Only runs the debug dataset by default.

4. `make bench`:
Runs the benchmarking script on the data found in results.

5. `make test`:
Runs a verification/correctness test on all the data found in results.

6. `make format`:
Formates code according to [google's style guide](https://google.github.io/styleguide/cppguide.html).

7. `make docs`:
Generates doxygen documentation.

## SSH
MPI needs to be able to ssh into the machines specified by the hostfile.

1. Run `ssh-keygen` if it hasn't been done already.
2. Run `ssh-copy-id <user>@hpc02.clients.eikon.tum.de` to copy generated key to a hpc machine.
3. (Optional) 
    1. Take `example-config` from `blob/example-config` and place it into your `~/.ssh` folder. 
    2. Then replace `<USER>` with your username. 
    3. Rename the file to `config`.
