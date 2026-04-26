#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define MAX_LINE_LENGTH 1024
#define MAX_SORTED_OUTPUT 10

int get_row_count(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    int count = 0;
    char buffer[MAX_LINE_LENGTH];

    if (fgets(buffer, MAX_LINE_LENGTH, file) != NULL) {
        while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL) {
            count++;
        }
    }
    fclose(file);
    return count;
}

WaveformSample* load_data(const char *filename, int *num_samples) {
    int count = get_row_count(filename);
    if (count <= 0) {
        printf("Error: File not found or empty: %s\n", filename);
        return NULL;
    }

    WaveformSample *data = (WaveformSample*)malloc(count * sizeof(WaveformSample));
    if (!data) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        free(data);
        return NULL;
    }

    char buffer[MAX_LINE_LENGTH];
    if (fgets(buffer, MAX_LINE_LENGTH, file) == NULL) {
        printf("Error: Empty or invalid file: %s\n", filename);
        fclose(file);
        free(data);
        return NULL;
    }

    int i = 0;
    WaveformSample *ptr = data;

    while (i < count && fgets(buffer, MAX_LINE_LENGTH, file) != NULL) {
        int parsed = sscanf(buffer, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                            &ptr->timestamp,
                            &ptr->phase_A_voltage,
                            &ptr->phase_B_voltage,
                            &ptr->phase_C_voltage,
                            &ptr->line_current,
                            &ptr->frequency,
                            &ptr->power_factor,
                            &ptr->thd_percent);

        if (parsed != 8) {
            printf("Warning: Skipping malformed row %d in %s\n", i + 2, filename);
            continue;
        }

        ptr->is_clipped_A = 0;
        ptr->is_clipped_B = 0;
        ptr->is_clipped_C = 0;

        ptr->status_A = 0;
        ptr->status_B = 0;
        ptr->status_C = 0;

        ptr++;
        i++;
    }

    *num_samples = i;
    fclose(file);
    return data;
}

static void print_status_text(FILE *file, uint8_t status) {
    if (status == 0) {
        fprintf(file, "OK");
        return;
    }

    if (status & FLAG_CLIPPING) fprintf(file, "CLIPPING ");
    if (status & FLAG_OUT_TOLERANCE) fprintf(file, "OUT_OF_TOLERANCE ");
}

void export_results(const char *filename, const char *source_file, PhaseMetrics *mA, PhaseMetrics *mB, PhaseMetrics *mC) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot create results file: %s\n", filename);
        return;
    }

    fprintf(file, "=== Power Quality Report for %s ===\n", source_file);
    fprintf(file, "Phase | RMS (V) | P2P (V) | DC Offset (V) | Variance | Std Dev | Health Mask | Clipped | Compliant | Status\n");
    fprintf(file, "-----------------------------------------------------------------------------------------------------------------\n");

    fprintf(file, "  A   | %7.2f | %7.2f | %13.4f | %8.2f | %7.2f |    0x%02X     | %-7s | %-9s | ",
            mA->rms, mA->p2p, mA->dc_offset, mA->variance, mA->std_dev, mA->health_status,
            (mA->health_status & FLAG_CLIPPING) ? "YES" : "NO", mA->is_compliant ? "YES" : "NO");
    print_status_text(file, mA->health_status);
    fprintf(file, "\n");

    fprintf(file, "  B   | %7.2f | %7.2f | %13.4f | %8.2f | %7.2f |    0x%02X     | %-7s | %-9s | ",
            mB->rms, mB->p2p, mB->dc_offset, mB->variance, mB->std_dev, mB->health_status,
            (mB->health_status & FLAG_CLIPPING) ? "YES" : "NO", mB->is_compliant ? "YES" : "NO");
    print_status_text(file, mB->health_status);
    fprintf(file, "\n");

    fprintf(file, "  C   | %7.2f | %7.2f | %13.4f | %8.2f | %7.2f |    0x%02X     | %-7s | %-9s | ",
            mC->rms, mC->p2p, mC->dc_offset, mC->variance, mC->std_dev, mC->health_status,
            (mC->health_status & FLAG_CLIPPING) ? "YES" : "NO", mC->is_compliant ? "YES" : "NO");
    print_status_text(file, mC->health_status);
    fprintf(file, "\n\n");
    fprintf(file, "Health bitmask key: bit 0 / 0x01 = clipping, bit 1 / 0x02 = out of tolerance.\n\n");
    fclose(file);
    printf("Successfully wrote results for %s to %s\n", source_file, filename);
}

void export_sorted_samples(const char *filename, WaveformSample **sorted_ptrs, int num_samples, int phase, const char *phase_name) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        printf("Error: Cannot append sorted samples to results file: %s\n", filename);
        return;
    }

    fprintf(file, "Sorted Phase %s samples by voltage magnitude, ascending, using pointer sort.\n", phase_name);
    fprintf(file, "Showing up to %d largest values after sorting.\n", MAX_SORTED_OUTPUT);
    fprintf(file, "Timestamp | Voltage (V) | Magnitude (V)\n");
    fprintf(file, "-----------------------------------------\n");

    int output_count = (num_samples < MAX_SORTED_OUTPUT) ? num_samples : MAX_SORTED_OUTPUT;
    int start = num_samples - output_count;

    for (int i = num_samples - 1; i >= start; i--) {
        double voltage = get_sample_voltage(sorted_ptrs[i], phase);
        fprintf(file, "%9.4f | %11.4f | %13.4f\n",
                sorted_ptrs[i]->timestamp, voltage, fabs(voltage));
    }

    fprintf(file, "\n");
    fclose(file);
}