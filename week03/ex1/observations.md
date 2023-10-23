
# Exercise sheet week 03

## Exercise 01

Run your programs with multiple problem and machine sizes and measure speedup and efficiency. Consider using strong scalability, weak scalability, or both. Justify your choice.

![parallel-b](/parallel-b.png)

For the parallel program with blocking functions 8 ranks seem to be a good choice. Even for small problem sizes like 384 a strong decrease in the computation time is observed.

![parallel-nb](/parallel-nb.png)

The same applies for the parallel program with non-blocking functions. Even the overall numbers are smaller, the curve within the graph does not change.

![parallel-b-speedup](/parallel-b-speedup.png)

![parallel-nb-speedup](/parallel-nb-speedup.png)
