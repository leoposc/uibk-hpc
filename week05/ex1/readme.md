# Assignment 5

The goal of this assignment is to try some MPI debugging.

## Exercise 1 (3 Points)

This exercise consists in comparing blocking and non-blocking communication for the heat stencil application.

### Tasks

- Take a look at the provided example codes.
- For each example code, run through the usual debugging workflow
    1) enable compiler warnings
    2) check with sanitizers
    3) run with a debugging tool of your choice (a working installation of MUST on LCC3 is provided in `/scratch/c703429/software/must-1.9.1`)
- Regardless of whether any of these steps highlights an issue, elaborate on the settings/parameters you used and what kind of errors they should detect!
- Report any bugs or issues that were found and attempt to fix them
- Is there anything else that can be done to highlight or avoid bugs?
- If available, use the same debugging workflow for one/some of your own implementations that exhibited strange behavior and see if you can spot any issues!

## Problems notices by executing the programs
Both code examples seem to run into a deadlock or something similar, since they dont finish.

## Enabling compiler warinings

To enable the compiler warnings we added the following flags to the compilation cycle.
This already showed two warning, in example1 a unused variable and in example2 a assignment to a variable inside a function
call. So in example2 this leads definetly to a unexpected behaviour.

Added flags:
`-Wall -Wextra -pedantic`

Resulting warning from example1.c:
```
example_1.c: In function 'main':
example_1.c:13:9: warning: unused variable 'i' [-Wunused-variable]
   13 |     int i;
      |
```

Resulting warning from example2.c:
```
example_2.c: In function 'main':
example_2.c:32:28: warning: operation on 'rank' may be undefined [-Wsequence-point]
   32 |                      (rank = 1 + size) % size, 123, MPI_COMM_WORLD, &status);
      |
```

## Check with sanitizers

After fixing the issues of the compiler warinings and running the code after compiling with sanitizers it lead to no additional warnings/issues.
Flags for sanitizer: `-fsanitize=undefined,address `

## Running with mustrun