# Assignment 5

## Exercise 2 (1 Point)

### Description of the Problem

The N-body problem is a common simulation pattern in scientific computing. Generally speaking, it models a system that evolves over time, where at each step in time, every element in the system updates its state based on the interaction between itself and every other element. This is in contrast with stencil applications, where the state is updated on elements only in close proximity. In terms of parallelization, this requires a different approach:

* Since for each particle we have an interaction with every other particle, there is no inherent link between performance and geometric location of the element in a system (as opposed to stencil applications).

* To update the state of an element, we will eventually need the previous state of every other element. Thus, we need a mechanism to distribute results over multiple ranks.

However, there are also similarities between N-body and stencil applications:

* There is synchronization at every time step.

Some implications and observations for the concrete gravitational N-body problem:

* The interaction between a body and every other body is a linear combination. This means, that we can compute the interaction between one group of particles and then with another group of particles. This also means, that we can start computing on values that are present and continue when we receive new problems. This is especially useful for parallelization.

### Optimization opportunities for the Sequential Approach

Some suggestions:

* Simplify the math. Check if some terms cancel out.
* Check if we have good cache hit rates.
* Try array of structs vs. struct of arrays.
* Check if we can reuse some computation. See https://en.wikipedia.org/wiki/Computational_complexity_of_matrix_multiplication.

### Parallel Approaches

To improve simulation performance, we want to parallelize our N-body problem and make it available for a MPI environment. Essentially, the goal is to distribute the computation across multiple ranks as efficiently as possible. So, we want to distribute the work equally, efficiently, and at the same time reduce the communication overhead as much as possible.

Similar to stencil applications, we argue that it would be a sensible approach to assign to each rank a sub array of the entire system. This rank updates for each time step the state of its elements. However, since we are on a distributed system, that way, the state of the simulation is distributed. Since we will need access to the state of every other element of the previous time step, can have to a communication pattern, that ensures that ever rank will eventually have access to the collected state of the previous iteration.

### Parallel Approach 1: Central Communication

The simplest solution we would come up was to appoint a single rank that, in addition to computation, handles the gathering and distribution of the distributed state. At the end of each time step, this rank requests the local state of every other rank, collects them into a single array, and then redistributes the collected array to the other ranks. Once a rank is done with the computation, they will wait for this array, store it locally, and use it to update the state of its assigned elements.

Useful MPI commands:

* MPI_Gather: used to collect the local results
* MPI_Bcast: used to send and receive the collected state

Pros:

* Simple and straight forward
* Not a lot of adjustment from the sequential application
* Few, but bulky send/receive operations. Might be more efficient than many, small send/receive operations

Cons:

* No latency hiding
* Overloaded communication rank, which results in busy waiting of the other ranks

### Parallel Approach 2: Mesh Communication

As a second approach, one could consider using some kind of mesh communication to distribute the shared state. Opposed to using a single node that handles the distribution of state, we could implement the computational ranks in such a way, that they broadcast their results to every other rank in the application once they are finished. In that case, we could also use non-blocking communication and latency hiding. We first start a non-blocking receive for the other ranks, compute the state using the local state, wait for the other states to come in and compute again, broadcast the local state to every other rank and start the next iteration.

Useful MPI commands:

* MPI_Bcast: Used to send the collected state. Maybe we have to use a regular send here.
* MPI_Waitany: Periodically wait until we get a new state, then recompute.

Pros:

* Latency hiding
* Smaller exchanges

Cons:

* More complex communication
* More exchanges

### Additional considerations

* Do we have to send the entire state (all properties of the bodies, or is position enough)?
  
* There is a symmetry between the forces acting on two particles: `F_ij = -eF_ji`. Currently, we compute them twice, which is redundant. Maybe there are some interesting communication patterns such that we only have to compute half of the forces and reconstruct the other half.
