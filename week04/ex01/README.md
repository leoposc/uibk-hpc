### Team: Peter Burger, Leo Schmid, Fabian Aster

# Exercise sheet week 03

## Exercise 01

**Do a detailed performance analysis on your 1D and 2D heat stencil implementation. Use either tools discussed in the lecture (perf, gprof, gperftools, etc.) or any tools you deem fit for generating a performance profile.**

Tools considered:

* *gprof*: We had some trouble setting up gprof. Also, after some research, we found that `gprof` is no longer suitable for modern-day applications and that you should use something else (See https://gernotklingler.com/blog/gprof-valgrind-gperftools-evaluation-tools-application-level-cpu-profiling-linux/, https://stackoverflow.com/questions/1777556/alternatives-to-gprof).

* *perf*: We mainly used `perf`, as it seems to be the most capable and the most convenient

TODO

**Provide a report that discusses the most expensive source code locations ("hot spots") along with explaining why they are expensive and how to possibly improve on that. Compare blocking and non-blocking if possible, as well as 1D and 2D.**

TODO

**Consider improving your 2D stencil using the gathered information. If you do, document the improvement you managed to achieve. If you don't, argue why the application performance cannot be feasibly improved in your opinion.**

Step 0 - Improve Code Base:

* *Restructure Code Layout*: To make our code base more manageable, we did the following:
    * make sure that file names are consistent and meaningful
    * move the source code into the src/ directory
    * compile the binaries into the bin/ directory
    * outsource duplicate code used in multiple implementations into respective header files
        * `src/common/common1d.h`
        * `src/common/common2d.h`
    * adjust the Makefile accordingly
        * use mpicc from `module load openmpi/3.1.6-gcc-12.2.0-d2gmn55`
        * to build all, run `make`
        * to clean all, run `make clean`
* *Consistent Code Formatting*: To make the source code more consistent and improve readability, we use `clang-format`. To run clang-format, first load the module from `module load llvm/15.0.4-python-3.10.8-gcc-8.5.0-bq44zh7`. Then, to format all the source code in the project, run the script `bash format.sh`.
* *Consistent Benchmarking*: To simplify benchmarking, reduce the manual execution of multiple tasks, and ease the administration overhead for the benchmark results, we developed a small "benchmark suite". To benchmark multiple tasks, one can use the `benchmark.sh` script. First, adjust your parameters in the script (executables, ranks, problem sizes), then run the script using `bash benchmark.sh`. The script will then create and run SLURM jobs for each combination of parameters. The results are stored under `benchmarks/<ts>/<executable>_<ranks>_<problemsize>.out`, where `ts` is the UNIX time in seconds when the benchmark was executed and `executable`, `ranks` and `problemsize` are a combination of parameters given in the script.
* *Improve Reproducibility*: To improve reproducibility, we implemented most of the common workflows using bash scripts and documented their usage accordingly:
    * `make`
    * `make clean`
    * `bash format.sh`
    * `bash benchmark.sh`

Step 1: use sanitizer?

TODO

**Insert the final walltime and speedup of the 2D stencil for 96 cores for N=768x768 and T=768x768x100 into the comparison spreadsheet: https://docs.google.com/spreadsheets/d/18WIigEWPM3htroCkLbLoiVKf2x4J2PtxDbtuYUPTRQQ/edit**

TODO
