
# Task1

I think in my plot the points go out of scope and dont group up is,
because i didnt account for collisions mid way.
I didnt account for that, because its not possible with the simple formulas on the
help sheet so i assumed we dont have to. I only merge two particles when they are close enough after a iteration.

## Performance

1 Thread:
time: 109.679981  
2 Thread:         
time: 55.403915   
3 Thread:         
time: 37.291827   
4 Thread:         
time: 28.209613   
5 Thread:         
time: 24.208850   
6 Thread:         
time: 20.823560   
7 Thread:         
time: 18.661989   
8 Thread:         
time: 16.887523   
serial version    
time: 109.681131  
                  
On my laptop the difference wasnt nearly as big between serial and parallel.
I think thats because the computations are faster, so that the overhead of
creating the threads has a bigger impact.
When executing with 1000 points and 100 frames the serial version was
actually faster.
