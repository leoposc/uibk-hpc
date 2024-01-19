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

	// create the SYCL queue that handles task submission and synchronization
	// between host and device. since we dont use any other arguments here, it
	// will select the one that fits best. in our case, it will probably use
	// the OpenMP backend
	cl::sycl::queue q;

 	// create an explicit scope here. that way, we make sure that the data
	// is moved from the device to the host when the computation is finished
	// make sure to create the buffers in this scope. alternatively,
	// we could also call some explicit routines to move the data from device
	// to the host.
	{

		// create the buffers used to map memory between host and device
		// when these buffers are destructed, the data is automatically moved to the host device
		// during that time, we cannot access the values on the host side, as the changes
		// are only on the device
		// initialize it with the values from the above memory regions of the host
		cl::sycl::buffer<Datatype> buf_a(domain_a.data(), cl::sycl::range<1>(size_domain));
		cl::sycl::buffer<Datatype> buf_b(domain_b.data(), cl::sycl::range<1>(size_domain));

		time_start = std::chrono::system_clock::now();

		// iterate over every step in time
		// explicitly synchronize at the end of every step
		for (std::size_t t = 0; t < timesteps; t++) {

			// submit the parallel task for a single iteration
			q.submit([&](cl::sycl::handler& cgh) {

				// configure the access on the buffers. SYCL will use this information
				// to synchronize access accordingly
				auto r_a = buf_a.get_access<cl::sycl::access::mode::read>(cgh);
				auto w_b = buf_b.get_access<cl::sycl::access::mode::write>(cgh);
	
				// specify the kernel that computes a single element of the parallel for loop
				// this is declarative. SYCL will handle the actual placement of tasks for us.
				// note: we have to manually do the offset
				cgh.parallel_for<class StencilKernel>(cl::sycl::range<1>(size_domain - 2),
					[=](cl::sycl::item<1> item) {
						auto x = item.get_id(0) + 1;

						if (x == source_x) {
							w_b[x] = r_a[x];
							return;
						}

						Datatype value_left = r_a[x - 1];
						Datatype value_center = r_a[x];
						Datatype value_right = r_a[x + 1];

						w_b[x] = value_center + 0.2 * (value_left + value_right + (-2.0 * value_center));
					}
				);
				
			});

			// wait for the task iteration to finish
			q.wait();

			// pointer swap
			// this is valid, since we do not change the data
			// we just change the pointers usind in the
			// next interation
			auto tmp = std::move(buf_a);
			buf_a = std::move(buf_b);
			buf_b = std::move(tmp);

			// since we do not have the data on the host device, this does not make sense
			//if ((t % 10000) == 0) {
			//	std::cout << "Step t=" << t << "\t";
			//	printTemperature(domain_a);
			//	std::cout << std::endl;
			//}
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