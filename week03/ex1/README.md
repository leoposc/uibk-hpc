### Team: Peter Burger, Leo Schmid, Fabian Aster

# Exercise sheet week 03

## Exercise 01

**Run your programs with multiple problem and machine sizes and measure speedup and efficiency. Consider using strong scalability, weak scalability, or both. Justify your choice.**

![parallel-b](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-b.png?raw=true)

For the parallel program with blocking functions 8 ranks seem to be a good choice. Even for small problem sizes like 384 a strong decrease in the computation time is observed.

![parallel-nb](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-nb.png)

The same applies for the parallel program with non-blocking functions. Even the overall numbers are smaller, the curve within the graph does not change.

![parallel-b-speedup](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-b-speedup.png)

![parallel-nb-speedup](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-nb-speedup.png)

Efficiency can be seen here, it is in the expected values and decreases with the number of processes.

![parallel-b-efficiency](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-b-efficiency.png)

![parallel-nb-efficiency](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-nb-efficiency.png)


**Measure and illustrate one domain-specific and one domain-inspecific performance metric. What can you observe?**

Domain specific:

![parallel-b-domainspec](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-b-domainspec.png)

![parallel-nb-domainspec](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-nb-domainspec.png)

Domain inspecific:

![parallel-b-indomain](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-b-indomain.png)

![parallel-nb-indomain](https://github.com/leoposc/uibk-hpc/blob/main/week03/ex1/parallel-nb-indomain.png)

**How can you verify the correctness of your applications?**

Print the 2D heat stencil for every 10.000 timestamp and compare if with the sequential version.

