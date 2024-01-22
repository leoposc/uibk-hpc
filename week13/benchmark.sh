#!/bin/bash

export OMP_NUM_THREADS=1

build/heat_stencil_1d_sycl 1024 50000 0
build/heat_stencil_1d_sycl 1024 50000 1
build/heat_stencil_1d_sycl 2048 50000 0
build/heat_stencil_1d_sycl 2048 50000 1
build/heat_stencil_1d_sycl 4096 50000 0
build/heat_stencil_1d_sycl 4096 50000 1
build/heat_stencil_1d_sycl 8192 50000 0
build/heat_stencil_1d_sycl 8192 50000 1
build/heat_stencil_1d_sycl 16384 50000 0
build/heat_stencil_1d_sycl 16384 50000 1
build/heat_stencil_1d_sycl 32768 50000 0
build/heat_stencil_1d_sycl 32768 50000 1
build/heat_stencil_1d_sycl 65536 50000 0
build/heat_stencil_1d_sycl 65536 50000 1

export OMP_NUM_THREADS=4

build/heat_stencil_1d_sycl 1024 50000 0
build/heat_stencil_1d_sycl 1024 50000 1
build/heat_stencil_1d_sycl 2048 50000 0
build/heat_stencil_1d_sycl 2048 50000 1
build/heat_stencil_1d_sycl 4096 50000 0
build/heat_stencil_1d_sycl 4096 50000 1
build/heat_stencil_1d_sycl 8192 50000 0
build/heat_stencil_1d_sycl 8192 50000 1
build/heat_stencil_1d_sycl 16384 50000 0
build/heat_stencil_1d_sycl 16384 50000 1
build/heat_stencil_1d_sycl 32768 50000 0
build/heat_stencil_1d_sycl 32768 50000 1
build/heat_stencil_1d_sycl 65536 50000 0
build/heat_stencil_1d_sycl 65536 50000 1

export OMP_NUM_THREADS=8

build/heat_stencil_1d_sycl 1024 50000 0
build/heat_stencil_1d_sycl 1024 50000 1
build/heat_stencil_1d_sycl 2048 50000 0
build/heat_stencil_1d_sycl 2048 50000 1
build/heat_stencil_1d_sycl 4096 50000 0
build/heat_stencil_1d_sycl 4096 50000 1
build/heat_stencil_1d_sycl 8192 50000 0
build/heat_stencil_1d_sycl 8192 50000 1
build/heat_stencil_1d_sycl 16384 50000 0
build/heat_stencil_1d_sycl 16384 50000 1
build/heat_stencil_1d_sycl 32768 50000 0
build/heat_stencil_1d_sycl 32768 50000 1
build/heat_stencil_1d_sycl 65536 50000 0
build/heat_stencil_1d_sycl 65536 50000 1

export OMP_NUM_THREADS=12

build/heat_stencil_1d_sycl 1024 50000 1
build/heat_stencil_1d_sycl 2048 50000 0
build/heat_stencil_1d_sycl 2048 50000 1
build/heat_stencil_1d_sycl 4096 50000 0
build/heat_stencil_1d_sycl 4096 50000 1
build/heat_stencil_1d_sycl 8192 50000 0
build/heat_stencil_1d_sycl 8192 50000 1
build/heat_stencil_1d_sycl 16384 50000 0
build/heat_stencil_1d_sycl 16384 50000 1
build/heat_stencil_1d_sycl 32768 50000 0
build/heat_stencil_1d_sycl 32768 50000 1
build/heat_stencil_1d_sycl 65536 50000 0
build/heat_stencil_1d_sycl 65536 50000 1
