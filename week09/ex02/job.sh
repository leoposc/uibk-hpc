#!/bin/bash

sudo -S perf stat ./a.out %* -a -e "power/energy-cores/" 