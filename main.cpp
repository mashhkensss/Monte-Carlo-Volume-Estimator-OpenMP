#include <iostream>
#include <fstream>
#include <cstdlib>
#include <omp.h>
#include <cstring>
#include <random>
#include "stdio.h"
#include "hit.h"

const float xhh = 1.0f / 4294967296.0f;

class Xorshift {
public:
    Xorshift(unsigned seed) {
        state = seed;
    }

    float next() {
        unsigned x = state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        state = x;
        return x ;
    }

    float next_float(float min, float d) {
        return min + d * next() * xhh;
    }

private:
    unsigned state;
};


void calculate_volume(int num_threads, int num_points, const char* output_file, bool use_omp) {
    const float range[6] = {get_axis_range()[0], get_axis_range()[1], get_axis_range()[2], get_axis_range()[3],
                            get_axis_range()[4], get_axis_range()[5]};
    float x_min = range[0], x_max = range[1];
    float y_min = range[2], y_max = range[3];
    float z_min = range[4], z_max = range[5];

    float xd = x_max - x_min;
    float yd = y_max - y_min;
    float zd = z_max - z_min;

    int hit_count = 0;
    double start_time = omp_get_wtime();

    if (use_omp) {
#pragma omp parallel num_threads(num_threads)
        {
            unsigned seed = time(NULL) + omp_get_thread_num();
            Xorshift gen(seed);
            int local_hit_count = 0;

#pragma omp for schedule(static, 100000)
            for (int i = 0; i < num_points; ++i) {
                float x = gen.next_float(x_min, xd);
                float y = gen.next_float(y_min, yd);
                float z = gen.next_float(z_min, zd);

                if (hit_test(x, y, z)) {
                    ++local_hit_count;
                }
            }

#pragma omp atomic
            hit_count += local_hit_count;
        }
    } else {
        unsigned seed = time(NULL) + omp_get_thread_num();
        Xorshift gen(seed);

        for (int i = 0; i < num_points; ++i) {
            float x = gen.next_float(x_min, xd);
            float y = gen.next_float(y_min, yd);
            float z = gen.next_float(z_min, zd);

            if (hit_test(x, y, z)) {
                ++hit_count;
            }
        }
    }

    double end_time = omp_get_wtime();
    float volume = static_cast<float>(hit_count) / static_cast<float>(num_points) * xd * yd * zd;

    FILE* output = fopen(output_file, "w");
    fprintf(output, "%g\n", volume);
    fclose(output);
    printf("Time (%i thread(s)): %g ms\n", (use_omp ? num_threads : 0), (end_time - start_time) * 1000.0f);
}

int main(int argc, char* argv[]) {
    bool use_omp = true;
    int num_threads = omp_get_max_threads();
    const char* input_file = nullptr;
    const char* output_file = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--no-omp") == 0) {
            use_omp = false;
            num_threads = 1;
        } else if (std::strcmp(argv[i], "--omp-threads") == 0) {
            if (i + 1 < argc && std::strcmp(argv[i + 1], "default") != 0) {
                num_threads = std::atoi(argv[++i]);
            }
        } else if (std::strcmp(argv[i], "--input") == 0) {
            if (i + 1 < argc) {
                input_file = argv[++i];
            }
        } else if (std::strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
        }
    }

    if (input_file == nullptr) {
        std::cerr << "Error: Input file not provided.\n";
        return 1;
    }

    if (output_file == nullptr) {
        std::cerr << "Error: Output file not provided.\n";
        return 1;
    }

    int num_points = 0;
    std::ifstream input(input_file);
    if (input.is_open()) {
        input >> num_points;
        input.close();
    } else {
        std::cerr << "Error: Could not open input file.\n";
        return 1;
    }

    if (num_points <= 0) {
        std::cerr << "Error: Number of points must be a positive integer.\n";
        return 1;
    }

    calculate_volume(num_threads, num_points, output_file, use_omp);
    return 0;
}
