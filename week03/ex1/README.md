
# Exercise sheet week 03

## Exercise 01

**Run your programs with multiple problem and machine sizes and measure speedup and efficiency. Consider using strong scalability, weak scalability, or both. Justify your choice.**

![parallel-b](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-b.png?raw=true)
<!-- ![parallel-b](/parallel-b.png) -->

For the parallel program with blocking functions 8 ranks seem to be a good choice. Even for small problem sizes like 384 a strong decrease in the computation time is observed.

![parallel-nb](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-b.png)

The same applies for the parallel program with non-blocking functions. Even the overall numbers are smaller, the curve within the graph does not change.

![parallel-b-speedup](/parallel-b-speedup.png)

![parallel-nb-speedup](/parallel-nb-speedup.png)

**Measure and illustrate one domain-specific and one domain-inspecific performance metric. What can you observe?**



**How can you verify the correctness of your applications?**

Print the 2D heat stencil for every 10.000 timestamp and compare if with the sequential version.

