### Team: Peter Burger, Leo Schmid, Fabian Aster, Marko Zaric
# Assignment 9

## Exercise 1

In order to find out the CPU models on Linux we used *lscpu* which resulted in:

* AMD Ryzen 7 3800X 8-Core Processor
* Haswell Intel® Core™ i7-4770 CPU @ 3.40GHz × 8
* Intel(R) Core(TM) i7-1065G7 CPU @ 1.30GHz x 8

To find out which energy or power instrumentation capabilities the system provides one can run:

    $sudo sensor-detect

    $sensors

This producess the following output for the AMD Ryzen 7 CPU:

    iwlwifi_1-virtual-0
    Adapter: Virtual device
    temp1:        +50.0°C  
    
    k10temp-pci-00c3
    Adapter: PCI adapter
    Tctl:         +65.2°C  
    Tccd1:        +68.8°C  
    
The output of this on the Intel(R) Core(TM) i7-1065G7 is this:

    coretemp-isa-0000
    Adapter: ISA adapter
    Package id 0:  +51.0°C  (high = +100.0°C, crit = +100.0°C)
    Core 0:        +45.0°C  (high = +100.0°C, crit = +100.0°C)
    Core 1:        +45.0°C  (high = +100.0°C, crit = +100.0°C)
    Core 2:        +50.0°C  (high = +100.0°C, crit = +100.0°C)
    Core 3:        +47.0°C  (high = +100.0°C, crit = +100.0°C)

    nvme-pci-5900
    Adapter: PCI adapter
    Composite:    +43.9°C  

    BAT0-acpi-0
    Adapter: ACPI interface
    in0:          16.49 V  

    iwlwifi_1-virtual-0
    Adapter: Virtual device
    temp1:        +38.0°C  

    hp-isa-0000
    Adapter: ISA adapter
    fan1:           0 RPM
    fan2:           0 RPM

    nvme-pci-5800
    Adapter: PCI adapter
    Composite:    +35.9°C  (low  =  -0.1°C, high = +76.8°C)
                         (crit = +79.8°C)

    acpitz-acpi-0
    Adapter: ACPI interface
    temp1:        +53.0°C

>During the *sensors-detect* we found that the driver **k10temp** is linked to the AMD Family 17h thermal sensors which means this Tctl and Tccd1 are the CPU temperatures.

>TCTL: This is the primary temperature reported by the CPU. Quote from [k10temp]: "Tctl is the processor temperature control value, used by the platform to control cooling systems. Tctl is a non-physical temperature on an arbitrary scale measured in degrees. It does \_not\_ represent an actual physical temperature like die or case temperature. Instead, it specifies the processor temperature relative to the point at which the system must supply the maximum cooling for the processor's specified maximum case temperature and maximum thermal power dissipation." 

>The resolution on the [k10temp] website is stated to be 1/8th of a degree there is no mention about the accuracy. 

> The Laptop with Intel(R) Core(TM) i7-1065G7 provides multiple sensors, like the battery voltage, two temperatures of pci connectors, the speed of the fans, acpitz-acpi-0 which is the temperature of the power interface and all the coretemps under coretemp-isa-0000

Running perf on a *Intel(R) Core(TM) i7-1065G7*/ Operating System: *Linux Ubuntu*


    sudo perf stat -a -e "power/energy-cores/" /bin/ls

    >> Performance counter stats for 'system wide':
    >>
    >> 0,01 Joules power/energy-cores/                                                   
    >>
    >> 0,001088134 seconds time elapsed

    sudo perf stat -a -e "power/energy-cores/" /bin/mkdir test

    >> Performance counter stats for 'system wide':
    >>
    >> 0,01 Joules power/energy-cores/
    >>
    >> 0,000961092 seconds time elapsed

Running perf on a *Haswell Intel® Core™ i7-4770 CPU @ 3.40GHz × 8*/ Operating System: *Linux Ubuntu*


    sudo perf stat -a -e "power/energy-cores/" /bin/ls

    >> performance counter stats for 'system wide':
    >>
    >> 0,00 Joules power/energy-cores/
    >>
    >> 0,000745074 seconds time elapsed

    sudo perf stat -a -e "power/energy-cores/" /bin/mkdir test

    >> performance counter stats for 'system wide':
    >>
    >> 0,01 Joules power/energy-cores/
    >>
    >> 0,000770920 seconds time elapsed


> The power/energy-cores/ perf counter is based on an MSR register called MSR_PP0_ENERGY_STATUS, which is part of the Intel RAPL interface. RAPL is a powerful tool to not only measure, but also limit power supply.
>
> PP0 refers to *power plane 0*, a RAPL domain containing all cores of the socket and private caches of cores. Not included is the last-level cache, the interconnect, memory controller, the GPU and other auxiliary components not directly being part of the core. Apparently there is no possibility to measure PP0 in any other way, which makes it difficult to say something about the accurancy.
>
> It is possible to measure other RAPL domains like the Package, DRAM and PSys domains.
>
> -- <cite>https://stackoverflow.com/questions/55956287/perf-power-consumption-measure-how-does-it-work</cite>
>
> -- <cite>https://dl.acm.org/doi/10.1145/3177754</cite>

Running perf on a *AMD Ryzen 7 3800X 8-Core Processor*/ Operating System: *Linux Ubuntu*:
    sudo perf stat -a -e power/energy-pkg/ /bin/ls

    >> Performance counter stats for 'system wide':
    >> 
    >> 0,00 Joules power/energy-pkg/                                           
    >>
    >> 0,000810670 seconds time elapsed

    sudo perf stat -a -e power/energy-pkg/ /bin/mkdir test

    >> Performance counter stats for 'system wide':
    >> 
    >> 0,05 Joules power/energy-pkg/                                          
    >>
    >> 0,000869161 seconds time elapsed

Running perf on a *Intel(R) Core(TM) i7-1065G7*/ Operating System: *Linux Ubuntu* with power/energy-pkg/
    sudo perf stat -a -e power/energy-pkg/ /bin/ls

    >> Performance counter stats for 'system wide':
    >> 
    >> 0,02 Joules power/energy-pkg/                                                     
    >> 
    >> 0,001159079 seconds time elapsed

    sudo perf stat -a -e power/energy-pkg/ /bin/mkdir test

    >> Performance counter stats for 'system wide':
    >> 
    >> 0,01 Joules power/energy-pkg/                                                     
    >> 
    >> 0,000887226 seconds time elapsed



>The AMD Ryzen did not support *power/energy-cores* therefore we had to run *power/energy-pkg/* on it with perf. The main diffence according to [lwn.net] is that *power/energy-cores/* measures the power consumption of all cores on socket without LLC cache and *power/energy-pkg* includes it. 


## Exercise 2

#### Busy waiting: 

    >> [sudo] Passwort für leopold: [sudo] Passwort für leopold: [sudo] Passwort für 
    >> leopold: [sudo] Passwort für leopold:
    >> Parallel programm finished. 1 processes ran in total.
    >> Computed 3.141459 for Pi
    >> Time ellapsed: 5.127953

    >> Performance counter stats for 'system wide':

    >>     90,33 Joules power/energy-cores/                                         

    >>     5,160884818 seconds time elapsed

    >> ^C[mpiexec@LeosUbuntu] Sending Ctrl-C to processes as requested
    >> [mpiexec@LeosUbuntu] Press Ctrl-C again to force abort

#### Yielding:

    mpirun -np 4 ./job.sh

    >> [sudo] Passwort für leopold: [sudo] Passwort für leopold: [sudo] Passwort für
    >> leopold: [sudo] Passwort für leopold:
    >> Parallel programm finished. 1 processes ran in total.
    >> Computed 3.141718 for Pi
    >> Time ellapsed: 4.831106
    >>
    >> Performance counter stats for 'system wide':
    >>
    >>     74,17 Joules power/energy-cores/                                         
    >>
    >>     4,870104016 seconds time elapsed
    >>
    >> ^C[mpiexec@LeosUbuntu] Sending Ctrl-C to processes as requested
    >> [mpiexec@LeosUbuntu] Press Ctrl-C again to force abort

The measured time is way more stable when the ranks are yielding instead of busy waiting, which does make sense since the program claims all available cores. What probably happens is that the scheduler is pausing the busy waiting threads as well to schedule other basic operating tasks. So the the execution time of the *busy waiting-program* depends on the execution time of other tasks, which certainly can differ from time to time. Bottom line is that the execution time for the *busy waiting-program* was between 5.0 and 5.4 seconds, for the *yielding-program* it was more constant around 4.8 seconds

### Oversubscribing

#### Busy waiting:



#### Yielding:

    mpirun -np 8 ./job.sh
    
    >> Parallel programm finished. 1 processes ran in total.
    >> Computed 3.141635 for Pi
    >> Time ellapsed: 5.128428
    >> 
    >>  Performance counter stats for 'system wide':
    >> 
    >>             129,90 Joules power/energy-cores/                                         
    >> 
    >>        5,168110821 seconds time elapsed

^C[mpiexec@LeosUbuntu] Sending Ctrl-C to processes as requested
[mpiexec@LeosUbuntu] Press Ctrl-C again to force abort


## Exercise 3



[k10temp]: <https://www.kernel.org/doc/html/v5.8/hwmon/k10temp.html>


[lwn.net]: <https://lwn.net/Articles/573602/>