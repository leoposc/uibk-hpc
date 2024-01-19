#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <CL/sycl.hpp>

constexpr std::size_t output_resolution = 128;

using Datatype = double;
using Domain = std::vector<Datatype>;

void printTemperature(const Domain &domain);

int verifyTemperature(const Domain &domain);

void simulateStep(
	cl::sycl::queue q,
	cl::sycl::buffer<Datatype> buf_a,
	cl::sycl::buffer<Datatype> buf_b,
	std::size_t size_domain,
	std::size_t source_x
);

Domain fetchFromDevice(
	cl::sycl::queue q,
	cl::sycl::buffer<Datatype> buf_device,
	std::size_t size_domain
);

int main(int argc, char **argv) {

	std::size_t size_domain = 2048;
	std::size_t timesteps = 50000;
	if(argc > 1) {
		size_domain = std::stoul(argv[1]);
		if(argc > 2) {
			timesteps = std::stoul(argv[2]);
		}
	}

	std::cout << "Computing heat-distribution for room size N=" << size_domain << " for T=" << timesteps << std::endl;

	Domain domain_a = Domain(size_domain, 273.0);
	const std::size_t source_x = size_domain / 2;

	std::cout << "Heat Source is at " << source_x << std::endl;
	domain_a[source_x] = 273 + 60;

	std::cout << "Initial:\t";
	printTemperature(domain_a);
	std::cout << std::endl;

	Domain domain_b = Domain(size_domain, 273.0);

	std::chrono::time_point<std::chrono::system_clock> time_start, time_end;

	// Create the SYCL queue that handles task submission and synchronization
	// between the host and the device. Since we use the default constructor
	// here, it will select the one that fits best. On LCC3, this should be
	// the OpenMP backend.
	cl::sycl::queue q;

	// The destructor of a SYCL buffer will move its underlying data from the
	// device back to the host. We can enforce this by using an explicit scope.
	{

		// Create buffers to map memory between the host and the device.
		cl::sycl::buffer<Datatype> buf_a(domain_a.data(), size_domain);
		cl::sycl::buffer<Datatype> buf_b(domain_b.data(), size_domain);

		time_start = std::chrono::system_clock::now();
		
		for (std::size_t t = 0; t < timesteps; t++) {

			// Use the device to advance the simulation for the current time step.
			// This call is blocking. However, the data remanins on the device.
			simulateStep(q, buf_a, buf_b, size_domain, source_x);

			// Swap the pointers of the two buffers. This is a valid operation,
			// since we do not modify the values in the buffer. We just change the
			// order in which we access them in the next iteration.
			std::swap(buf_a, buf_b);

			// Periodically show the progress of the simulation. Since we have to print on
			// the host device, we first have to fetch the data from the device.
			if ((t % 10000) == 0) {
				std::cout << "Step t=" << t << "\t";
				printTemperature(fetchFromDevice(q, buf_a, size_domain));
				std::cout << std::endl;
			}
		}

		time_end = std::chrono::system_clock::now();
	}

	const std::chrono::duration<double> elapsed_seconds = time_end - time_start;

	std::cout << "\t\t";
	printTemperature(domain_a);
	std::cout << std::endl;
	int verification_result = EXIT_SUCCESS;
	verification_result = verifyTemperature(domain_a);
	std::cout << "Computation took " << elapsed_seconds.count() << " seconds" << std::endl;

	return verification_result;
}

void printTemperature(const Domain &domain) {
	const std::string colors = " .-:=+*^X#%@";

	constexpr Datatype max = 273 + 30;
	constexpr Datatype min = 273 + 0;

	// step size in each dimension
	const std::size_t step_size = domain.size() / output_resolution;

	// left border
	std::cout << "X";

	for (std::size_t i = 0; i < output_resolution; i++) {

		// get max temperature in this tile
		Datatype cur_max = 0;
		for (std::size_t x = step_size * i; x < step_size * i + step_size; x++) {
			cur_max = (cur_max < domain[x]) ? domain[x] : cur_max;
		}
		Datatype temp = cur_max;

		// pick the 'color'
		int c = ((temp - min) / (max - min)) * colors.length();
		c = (c >= static_cast<int>(colors.length())) ? colors.length() - 1 : ((c < 0) ? 0 : c);

		// print the average temperature
		std::cout << colors[c];
	}

	// right border
	std::cout << "X";
}

int verifyTemperature(const Domain &domain) {
	for (std::size_t x = 0; x < domain.size(); x++) {
		if (domain[x] < 273.0 || domain[x] > 273.0 + 60) {
			std::cout << "Verification failed, grid[" << x << "]=" << domain[x] << std::endl;
			return EXIT_FAILURE;
		}
	}
	std::cout << "Verification succeeded" << std::endl;
	return EXIT_SUCCESS;
}

void simulateStep(
	cl::sycl::queue q,
	cl::sycl::buffer<Datatype> buf_a,
	cl::sycl::buffer<Datatype> buf_b,
	std::size_t size_domain,
	std::size_t source_x)
{
	q.submit([&](cl::sycl::handler& cgh) {
		auto r_a = buf_a.get_access<cl::sycl::access::mode::read>(cgh);
		auto w_b = buf_b.get_access<cl::sycl::access::mode::write>(cgh);

		// Specify the kernel that computes a single element of the parallel loop.
		// SYCL will handle the parallelization for us. Since we do not consider
		// the elements 0 and size_domain - 2, we manually have to set the offset.
		cgh.parallel_for<class StencilKernel>(size_domain - 2, [=](cl::sycl::id<1> idx) {
			auto x = (std::size_t) idx + 1;

			if (x == source_x) {
				w_b[x] = r_a[x];
				return;
			}

			Datatype value_left = r_a[x - 1];
			Datatype value_center = r_a[x];
			Datatype value_right = r_a[x + 1];

			w_b[x] = value_center + 0.2 * (value_left + value_right + (-2.0 * value_center));
		});
	});

	// Wait for the computation to finish on the device.
	q.wait();
}

Domain fetchFromDevice(
	cl::sycl::queue q,
	cl::sycl::buffer<Datatype> buf_device,
	std::size_t size_domain)
{
	Domain result = Domain(size_domain);

	// By using an explicit scope here, we make sure that the
	// result is synchronized on the host.
	{

		// The buffer used to map memory between device and host.
		cl::sycl::buffer<Datatype> buf_result(result.data(), size_domain);

		q.submit([&](cl::sycl::handler& cgh){
			auto r_device = buf_device.get_access<cl::sycl::access::mode::read>(cgh);
			auto w_result = buf_result.get_access<cl::sycl::access::mode::write>(cgh);

			// Specify the kernel that simply copies the data from one buffer
			// to another buffer.
			cgh.parallel_for<class CopyKernel>(size_domain, [=](cl::sycl::id<1> x) {
				w_result[x] = r_device[x];
			});
		});

		// Wait for the copy to finish on the device.
		q.wait();
	}

	return result;
}