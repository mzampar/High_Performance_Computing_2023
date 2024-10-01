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
- Root node transmits to $P-1$ child nodes, without segmentation.

### Chain Tree Algorithm
- Internal nodes have one child.
- Messages are split into segments, transmitted in a pipeline.

### Binary Tree Algorithm
- Internal process has two children, then $P = 2^H -1$, assuming the tree is complete, with $H=\log_2(P+1)$ the height of the tree.
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



![](./Excercise_1/bcast/results/figures/model.png)


---

# Conclusion

The default algorithm doesn't always choose the best algorithm to perform the broadcast, expecially for node allocation, large message size and many processes, with a remarkable difference.

---

# Barrier Algorithms analysed

### Linear: 
- All nodes report to a preselected root
- Linear communication steps.

### Tree: 
- Hierarchical synchronization in a tree-like structure.
- Logarithmic communication steps.

### Recursive Doubling: 
- Optimal for powers of 2. At step $k$, node $r$ exchanges a message with node $(r \texttt{XOR} 2^k)$.
- Logarithmic communication steps.

---

## Latency against number of processes (Fixed allocation)

![](./Excercise_1/barrier/results/figures/latency_vs_processes.png)


---

## Latency against number of processes (Fixed algorithm)

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

OpenMPI's default algorithm doesn't always choose the optimal algorithm in terms of latency, but in the case of `MPI_Barrier` the difference is not remarkable.

---

# Thanks for your attention!