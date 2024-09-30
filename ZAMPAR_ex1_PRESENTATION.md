---
marp: true
title: High Performance Computing 2023 - Exercise 1
author: Marco Zampar
date: October 01, 2024
---

# High Performance Computing 2023 - Exercise 1
### Marco Zampar - SM3800032
October 01, 2024

---

# Problem Statement

This project aims to:
- Estimate the latency of the default OpenMPI collective operations Broadcast and Barrier.
- Vary the number of processes and message sizes for different allocations of the processes.
- Compare the results with other algorithms.
- Infer the underlying performance models of the algorithms.

---

# Computational Resources

- 2 THIN nodes from the ORFEO cluster.
- Each node has 2 CPUs with 12 cores in a single NUMA region.

---

# Broadcast Algorithms analysed

### Flat Tree Algorithm
- Single-level tree topology.
- Root node transmits to P-1 child nodes.

### Chain Tree Algorithm
- Internal nodes have one child.
- Messages are split into segments, transmitted in a pipeline.

### Binary Tree Algorithm
- Internal process has two children.
- Segmentation is used to improve communication parallelism.

---
### Latency vs Message Size (Fixed Processes)

![w:800](./Excercise_1/bcast/results/figures/latency_size_core.png)
![w:800](./Excercise_1/bcast/results/figures/latency_size_socket.png)
![w:800](./Excercise_1/bcast/results/figures/latency_size_node.png)

---

### Latency vs Number of Processes (Fixed Message Size and Core allocation)

![](./Excercise_1/bcast/results/figures/latency_proc_core_4.png)
![](./Excercise_1/bcast/results/figures/latency_proc_core_16.png)

---
### Latency vs Number of Processes (Fixed Message Size and Socket allocation)

![](./Excercise_1/bcast/results/figures/latency_proc_socket_4.png)
![](./Excercise_1/bcast/results/figures/latency_proc_socket_16.png)

---
### Latency vs Number of Processes (Fixed Message Size and Node allocation)

![](./Excercise_1/bcast/results/figures/latency_proc_node_4.png)
![](./Excercise_1/bcast/results/figures/latency_proc_node_16.png)

---
### Latency vs Number of Processes (Fixed Algorithm)

![](./Excercise_1/bcast/results/figures/latency_proc_1.png)
![](./Excercise_1/bcast/results/figures/latency_proc_16.png)


---
### Comparison of the Default algorithm with the others

![bg w:800 right:65%](./Excercise_1/bcast/results/figures/table.png)

---

# Performance Models

Linear regression models were used to estimate the latency surface, varying the number of processes and message sizes.

![](./Excercise_1/bcast/results/figures/model.png)


---

# Conclusion

- The default algorithm doesn't always choose the best algorithm to perform the broadcast.

---

# Barrier Algorithms analysed

### Linear: 
All nodes report to a root.

### Tree: 
Hierarchical synchronization in a tree-like structure.

### Recursive Doubling: 
Logarithmic communication steps, optimal for powers of 2.

---

## Latency against number of processes (fixing the allocation)

![](./Excercise_1/barrier/results/figures/latency_vs_processes.png)


---

## Latency against number of processes (fixing the algorithm)

![](./Excercise_1/barrier/results/figures/latency_fix_alg.png)

---
## Comparison of the Default algorithm with the others

![](./Excercise_1/barrier/results/figures/latency_diff.png)

---

![bg 100%](./Excercise_1/barrier/results/figures/latency_regression.png)
![bg 100%](./Excercise_1/barrier/results/figures/latency_spline.png)
![bg 100%](./Excercise_1/barrier/results/figures/latency_regression_1.png)


---

# Conclusion

- OpenMPI's default algorithm doesn't always choose the optimal one in terms of latency, but in the case of `MPI_Barrier` the difference is not remarkable.
