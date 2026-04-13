#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

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
    fgets(buffer, MAX_LINE_LENGTH, file);

    int i = 0;
    WaveformSample *ptr = data;
    while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL && i < count) {
        sscanf(buffer, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
               &(ptr->timestamp),
               &(ptr->phase_A_voltage),
               &(ptr->phase_B_voltage),
               &(ptr->phase_C_voltage),
               &(ptr->line_current),
               &(ptr->frequency),
               &(ptr->power_factor),
               &(ptr->thd_percent));

        ptr->status_A = 0;
        ptr->status_B = 0;
        ptr->status_C = 0;

        ptr++;
        i++;
    }

    *num_samples = count;
    fclose(file);
    return data;
}

void export_results(const char *filename, const char *source_file, PhaseMetrics *mA, PhaseMetrics *mB, PhaseMetrics *mC) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        printf("Error: Cannot create results file.\n");
        return;
    }

    fprintf(file, "=== Power Quality Report for %s ===\n", source_file);
    fprintf(file, "Phase | RMS (V) | P2P (V) | DC Offset (V) | Std Dev | Clipped | Compliant\n");
    fprintf(file, "-------------------------------------------------------------------------\n");
    fprintf(file, "  A   | %7.2f | %7.2f | %13.4f | %7.2f | %-7s | %-9s\n",
            mA->rms, mA->p2p, mA->dc_offset, mA->std_dev, mA->is_clipped ? "YES" : "NO", mA->is_compliant ? "YES" : "NO");
    fprintf(file, "  B   | %7.2f | %7.2f | %13.4f | %7.2f | %-7s | %-9s\n",
            mB->rms, mB->p2p, mB->dc_offset, mB->std_dev, mB->is_clipped ? "YES" : "NO", mB->is_compliant ? "YES" : "NO");
    fprintf(file, "  C   | %7.2f | %7.2f | %13.4f | %7.2f | %-7s | %-9s\n",
            mC->rms, mC->p2p, mC->dc_offset, mC->std_dev, mC->is_clipped ? "YES" : "NO", mC->is_compliant ? "YES" : "NO");
    fprintf(file, "\n");

    fclose(file);
    printf("Successfully wrote results for %s to %s\n", source_file, filename);
}