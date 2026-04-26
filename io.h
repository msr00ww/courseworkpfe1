#ifndef IO_H
#define IO_H

#include "waveform.h"

int get_row_count(const char *filename);
WaveformSample* load_data(const char *filename, int *num_samples);
void export_results(const char *filename, const char *source_file, PhaseMetrics *mA, PhaseMetrics *mB, PhaseMetrics *mC);
void export_sorted_samples(const char *filename, WaveformSample **sorted_ptrs, int num_samples, int phase, const char *phase_name);
static double get_voltage(WaveformSample *sample, int phase);
#endif