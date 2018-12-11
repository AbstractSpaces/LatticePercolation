# HPC Lattice Percolation v1.0

## Using the Program

percolation SIZE PROB THREADS MODE

* SIZE is the number of columns and rows in the square lattice, and must be a positive integer.
* PROB is the probability of bond formation and must be between 0.0 and 1.0 inclusive.
* THREADS is the number of threads to use in Test mode or the maximum number of threads in Compare mode. Must be a positive integer.
* MODE must be either "T" or "C".

## Background

This program started life as a university project for a high performance computing unit.
The original task was to demonstrate the use of OpenMP and MPI to speed up the analysis of [whether a 2D lattice would percolate](https://en.wikipedia.org/wiki/Percolation_theory).
This is a polished and re-implemented version of that project.

## Functional Overview

The program generates a 2D lattice with user specified size and bond probabilty. The lattice used is output to file for visual inspection.

When run in "Test" mode, the program analyses the lattice for percolation (utilising a user determined number of threads) and outputs the size and dimensions of the clusters found, and how long the program took to complete.

In "Compare" mode, the program performs the analysis repeatedly, incrementing the number of threads used up to a user specified limit.
It performs multiple runs and outputs the average time taken for each number of threads, as well as confirmation that every run found the same results and there were no errors.

## Implementation Overview

The original project was written in C, which I'm sticking with here as I want to show off some of the things I learned about the language.

The original project required the lattice to support clusters wrapping around to the other side when they reached the edge.
I'm omitting that now as it made things trickier without adding a whole lot to the learning exercise.
I'm also omitting the MPI part of the project as I don't have access to a system that can properly test that functionality.

## Developer Diary

As of right now, two things about my work here stand out to as substandard.

One is the complete lack of error handling, the program just exits whenever a system call doesn't return what it's supposed to.
If I had more time and energy to devote to the project, I'd make the effort to do multiple attempts and provide a more detailed error report.

The other is my Git workflow being kind of whack.
I'm still learning the dos and don'ts of version control and several times I've made changes in the wrong branch or written less than ideal commit messages.
But I try to be aware of when I've done a bad or made an amateur move, and hopefully will do better in future.

# Data Structure Designs

## Lattice

2D array, one element per lattice site.
Each element references the adjacent sites connected by bond.
Straightforward, intuitive, easy to visualise and implement.
Extra space consumed by two-way references and empty sites, outweighed by ease of implementation.

## Cluster

Used to track sub-clusters found in searching, and in stitching sub-clusters together.

Includes:

* Number of sites.
* Indices of lowest and highest columns and rows spanned by cluster.
* Vertical coordinates of any bonds crossing into the next lattice segment.

## Tracker

To allow the same structure being reused in Compare mode, the lattice itself should be immutable.
Tracking which sites have been seen and searched are done with a separate structure.
2D array with elements for each site. Sites marked either unseen, seen, or complete.

# Algorithm Designs

## Searching

All searches based on depth first graph search.

### Serial DFS

Iterative DFS, assembles one cluster at a time, storing and returning them in a list.
Recursive implementation caused stack overflow in the original project.

### Stitching DFS

Each thread DFS' a number of adjacent columns and returns a list of sub-clusters.
Sub-clusters stitched and final cluster list returned to caller.

## Stitching

Sub-clusters stitched 2 at a time by looking at the bonds crossing their border left to right.
The joined clusters are replaced with a new structure holding their summed values.
Can be done in parallel by giving each thread a pair of segments to stitch and iteratively turning pairs into a single segment until the lattice is whole again.