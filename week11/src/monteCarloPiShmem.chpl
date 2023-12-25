// inspired by https://github.com/chapel-lang/chapel/blob/main/test/exercises/MonteCarloPi/deitz/MonteCarloPi/dataParallelMonteCarloPi.chpl

use Random;
use Time;

// usage:
// n ... the total number of iterations
config const n = 10000;

var rs = new randomStream(real);
var sw = new stopwatch();

// configure the domain
var D = {1..n};

sw.start();

// although short, the following line of code does a lot.
// 
// essentially, we declare a parallel iteration over the
// domain D, where in each iteration we generate a
// random point (x, y) and add 1 to the counter c if it
// is in the unit circle.
//
// chapel automatically handels the parallelization for us.
var c = + reduce [(x, y) in zip(rs.iterate(D), rs.iterate(D))] x**2 + y**2 <= 1.0;

sw.stop();

// Print the result
writeln("pi: ", c * 4.0 / n);
writeln("time: ", sw.elapsed());
