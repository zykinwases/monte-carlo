#include <iostream>
#include <cmath>
#include <iomanip>
#include <cstdlib>
#include "mpi.h"

double F(double x, double y, double z) {
    if (x*x + y*y + z*z <= 1) return exp(x*x + y*y)*z;
    else return 0;
}

int main(int argc, char *argv[]) {
    double eps;
    double answer = (exp(1) - 2) * M_PI / 2;
    double result = 0;
    double sum = 0, stub = 0;
    int dots_for_one = 1024;
    int n = 0;
    int k;
    double time, start, max_time;
    bool stop = false;
    
    int rank, size;
    if (argc > 3 || argc < 2) {
        std::cerr << "Usage: monte_carlo.out <eps>" << std::endl;
        return 1;
    } 
    if ((eps = atof(argv[1])) <= 0) {
        std::cerr << "eps must be >= 0" << std::endl;
        return 1;
    } 
    if (argc == 3) {
        if (atoi(argv[2]) > 0) {
            dots_for_one = atoi(argv[2]);
        }
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 1) {
        std::cout << "Run consistent version" << std::endl;
        double x, y, z;
        start = MPI_Wtime();
        while (fabs(answer - result) >= eps) {
            x = (rand() * 1.0 / RAND_MAX) * 2.0 - 1;
            y = (rand() * 1.0 / RAND_MAX) * 2.0 - 1;
            z = (rand() * 1.0 / RAND_MAX);
            n++;
            sum += F(x, y, z);
            result = sum / n * 4;
        }
        time = MPI_Wtime() - start;
    } else {
        if (!rank) std::cout << "One master process and " << size-1 << " slave process(es)" << std::endl;
        start = MPI_Wtime();
        if (!rank) {
            double x[dots_for_one*(size-1)], y[dots_for_one*(size-1)], z[dots_for_one*(size-1)];
            MPI_Request req;
            while (fabs(answer - result) >= eps) {
                MPI_Bcast(&stop, 1, MPI_SHORT, 0, MPI_COMM_WORLD);
                for (int i = 0; i < dots_for_one*(size-1); i++) {
                    x[i] = (rand() * 1.0 / RAND_MAX) * 2.0 - 1;
                    y[i] = (rand() * 1.0 / RAND_MAX) * 2.0 - 1;
                    z[i] = (rand() * 1.0 / RAND_MAX);
                }
                n += dots_for_one*(size-1);
                for (int i = 1; i < size; i++) {
                    MPI_Isend(&x[dots_for_one*(i-1)], dots_for_one, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &req);
                    MPI_Isend(&y[dots_for_one*(i-1)], dots_for_one, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &req);
                    MPI_Isend(&z[dots_for_one*(i-1)], dots_for_one, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, &req);
                }
                MPI_Reduce(&stub, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
                result = sum / n * 4;
            }
            stop = true;
            MPI_Bcast(&stop, 1, MPI_SHORT, 0, MPI_COMM_WORLD);
            time = MPI_Wtime() - start;
        } else {
            double x[dots_for_one], y[dots_for_one], z[dots_for_one];
            MPI_Request req[3];
            MPI_Status stat[3];
            start = MPI_Wtime();
            while (1) {
                MPI_Bcast(&stop, 1, MPI_SHORT, 0, MPI_COMM_WORLD);
                if (stop) break;
                MPI_Irecv(x, dots_for_one, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &req[0]);
                MPI_Irecv(y, dots_for_one, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req[1]);
                MPI_Irecv(z, dots_for_one, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, &req[2]);
                MPI_Waitall(3, req, stat);
                for (int i = 0; i < dots_for_one; i++) {
                    sum += F(x[i], y[i], z[i]);
                }
                MPI_Reduce(&sum, &stub, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            }
            time = MPI_Wtime() - start;
        }
        MPI_Reduce(&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        time = max_time;
    }


    MPI_Finalize();
    
    if (!rank) {
        std::cout << "Programm result: " << std::setprecision(10) << result << std::endl;
        std::cout << "Counted error: " << std::setprecision(10) << fabs(answer - result) << std::endl;
        std::cout << "Dots count: " << n << std::endl;
        std::cout << "Time: " << time << std::endl;
    }

    return 0;
}