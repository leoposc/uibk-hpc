use Random;
use Time;

// usage:
// m ... the number of rows of the first matrix
// n ... the nubmer of columns in the first matrix
// l ... the number of columns in the second matrix
// v ... print the matrixes
// seed ... the seed for random nubmer generation
config const m = 1000;
config const n = 400;
config const l = 600;
config const v = false;
config const seed = 0;

var rs = new randomStream(real, seed);
var sw = new stopwatch();

var A: [1..m, 1..n] real;
var B: [1..n, 1..l] real;
var C: [1..m, 1..l] real;

rs.fill(A);
rs.fill(B);

if (v) {
    writeln("A");
    writeln(A);
    writeln("B");
    writeln(B);
}

sw.start();

// iterate over every column in B (parallel)
forall j in 1..l {

    // slice the column
    var B_j = B[.., j];

    // iterate over every row in A (sequential)
    for i in 1..m {

        // compute the dot product between row and column
        // and store the result in C
        C[i, j] = + reduce (A[i, ..] * B_j);
    }
}

sw.stop();

if (v) {
    writeln("C");
    writeln(C);
}

writeln("time: ", sw.elapsed());
