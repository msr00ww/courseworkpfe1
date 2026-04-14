#include <stdio.h>
#include <stdlib.h>
#include "waveform.h"
#include "io.h"

int main(int argc, char *argv[]) {
    int start_idx = 1;
    if (argc < 2) {
        printf("Usage: %s <file1.csv> [file2.csv ...]\n", argv[0]);
        printf("Attempting to load default: power_quality_log.csv\n\n");
    }

    int max_files = (argc > 1) ? argc : 2;

    remove("results.txt");

    for (int i = start_idx; i < max_files; i++) {
        const char *current_file = (argc > 1) ? argv[i] : "power_quality_log.csv";
        int num_samples = 0;

        WaveformSample *data = load_data(current_file, &num_samples);

        if (!data) {
            printf("\n[CLion Tip] If the file wasn't found, CLion is currently executing from its build directory.\n");
            printf("Make sure '%s' is placed inside the 'cmake-build-debug' folder, NOT just the main project folder!\n\n", current_file);
            continue;
        }

        printf("Processing %d samples from %s...\n", num_samples, current_file);

        PhaseMetrics metricsA, metricsB, metricsC;
        analyze_phase(data, num_samples, 0, &metricsA);
        analyze_phase(data, num_samples, 1, &metricsB);
        analyze_phase(data, num_samples, 2, &metricsC);

        export_results("results.txt", current_file, &metricsA, &metricsB, &metricsC);

        WaveformSample **sorted_ptrs = malloc(num_samples * sizeof(WaveformSample*));
        if (sorted_ptrs) {
            for (int j = 0; j < num_samples; j++) {
                sorted_ptrs[j] = &data[j];
            }
            perform_custom_sort(sorted_ptrs, num_samples, 0);
            free(sorted_ptrs);
        }

        free(data);
    }

    return EXIT_SUCCESS;
}