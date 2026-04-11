#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stdint.h>
#include <math.h>

#define NOMINAL_VOLTAGE 230.0
#define CLIPPING_THRESHOLD 324.9

#define FLAG_CLIPPING      (1 << 0)
#define FLAG_OUT_TOLERANCE (1 << 1)

typedef struct {
    double timestamp;
    double phase_A_voltage;
    double phase_B_voltage;
    double phase_C_voltage;
    double line_current;
    double frequency;
    double power_factor;
    double thd_percent;

    uint8_t status_A;
    uint8_t status_B;
    uint8_t status_C;
} WaveformSample;

typedef struct {
    double rms;
    double p2p;
    double dc_offset;
    double variance;
    double std_dev;
    int is_clipped;
    int is_compliant;
} PhaseMetrics;

void analyze_phase(WaveformSample *data, int num_samples, int phase, PhaseMetrics *metrics);
void perform_custom_sort(WaveformSample **ptrs, int num_samples, int phase);

#endif
