
# Exercise 1  
  
Overall the sequential program is faster, because there is not a lot of dense work done.  
The overhead created from the threads is not worth it.  
Thats also the bottleneck, it can partly be resolved by setting a threshold for parallel work.  
  
Ive found that a threshold of 5 is enough, more changes nothing and less slows the program down.  
  
time measurements:   
  
delannoy serial:  
num: 251595969  
time: 1.2010 seconds  
  
delannoy parallel 1 Thread:  
num: 251595969  
time: 10.0929 seconds  
  
delannoy parallel 2 Thread:  
num: 251595969  
time: 5.0512 seconds  
  
delannoy parallel 3 Thread:  
num: 251595969  
time: 3.9044 seconds  
  
delannoy parallel 4 Thread:  
num: 251595969  
time: 3.1270 seconds  
  
delannoy parallel 5 Thread:  
num: 251595969  
time: 2.5100 seconds  
  
delannoy parallel 6 Thread:  
num: 251595969  
time: 1.9362 seconds  
  
delannoy parallel 7 Thread:  
num: 251595969  
time: 1.7400 seconds  
  
delannoy parallel 8 Thread:  
num: 251595969  
time: 1.5255 seconds  
