all: monte_carlo

monte_carlo: monte_carlo.cpp
	mpicxx $^ -o $@

bluegene-one: monte_carlo
	for eps in 1.0e-4 2.0e-5 0.8e-5 ; do \
		mpisubmit.bg -n 1 -w 00:05:00 --stdout bg.1.$$eps.log $^ -- $$eps ; \
	done
	
polus-one: monte_carlo
	for eps in 3.0e-5 5.0e-6 1.5e-6 ; do \
		mpisubmit.pl -p 1 -w 00:05 --stdout pl.1.$$eps.log $^ -- $$eps ; \
	done

bluegene-all-1: monte_carlo
	for proc in 1 2 4 16 64 ; do \
		for eps in 1.0e-4 2.0e-5 ; do \
			for dots in 1 8 64 256 1024 ; do \
				mpisubmit.bg -n $$proc -w 00:30:00 --stdout bg.$$eps.$$proc.$$dots.log $^ -- $$eps $$dots ; \
			done \
		done \
	done \

bluegene-all-2: monte_carlo
	for proc in 1 2 4 16 64 ; do \
		for eps in 0.8e-5 1.0e-12 ; do \
			for dots in 1 8 64 256 1024 ; do \
				mpisubmit.bg -n $$proc -w 00:30:00 --stdout bg.$$eps.$$proc.$$dots.log $^ -- $$eps $$dots ; \
			done \
		done \
	done \

polus-all: monte_carlo
	for proc in 1 2 4 16 32 ; do \
		for eps in 3.0e-5 5.0e-6 1.5e-6 1.0e-12 ; do \
			for dots in 1 8 64 256 1024 ; do \
				mpisubmit.pl -p $$proc -w 00:30 --stdout pl.$$eps.$$proc.$$dots.log $^ -- $$eps $$dots; \
			done \
		done \
	done \