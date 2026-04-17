#include "waveform.h"
#include <stdlib.h>

static double get_voltage(WaveformSample *sample, int phase) {
    if (phase == 0) return sample->phase_A_voltage;
    if (phase == 1) return sample->phase_B_voltage;
    return sample->phase_C_voltage;
}
static void set_flags(WaveformSample *sample, int phase, uint8_t flag) {
    if (phase == 0) sample->status_A |= flag;
    else if (phase == 1) sample->status_B |= flag;
    else sample->status_C |= flag;
}

void analyze_phase(WaveformSample *data, int num_samples, int phase, PhaseMetrics *metrics) {
    if (!data || num_samples <= 0) return;

    double sum = 0.0;
    double sum_sq = 0.0;
    double max_val = get_voltage(data, phase);
    double min_val = get_voltage(data, phase);
    
    metrics->is_clipped = 0;
    WaveformSample *end = data + num_samples;
    for (WaveformSample *ptr = data; ptr < end; ptr++) {
        double v = get_voltage(ptr, phase);
        sum += v;
        sum_sq += (v * v);
        if (v > max_val) max_val = v;
        if (v < min_val) min_val = v;

        if (fabs(v) >= CLIPPING_THRESHOLD) {
            metrics->is_clipped = 1;
            set_flags(ptr, phase, FLAG_CLIPPING);
        }
    }
    metrics->dc_offset = sum / num_samples;
    metrics->rms = sqrt(sum_sq / num_samples);
    metrics->p2p = max_val - min_val;
    if (metrics->rms >= (NOMINAL_VOLTAGE * 0.9) && metrics->rms <= (NOMINAL_VOLTAGE * 1.1)) {
        metrics->is_compliant = 1;
    } else {
        metrics->is_compliant = 0;
        for (WaveformSample *ptr = data; ptr < end; ptr++) {
            set_flags(ptr, phase, FLAG_OUT_TOLERANCE);
        }
    }
    double var_sum = 0.0;
    for (WaveformSample *ptr = data; ptr < end; ptr++) {
        double diff = get_voltage(ptr, phase) - metrics->dc_offset;
        var_sum += (diff * diff);
    }
    metrics->variance = var_sum / num_samples;
    metrics->std_dev = sqrt(metrics->variance);
}
void perform_custom_sort(WaveformSample **ptrs, int num_samples, int phase) {
    WaveformSample **end = ptrs + num_samples;
    
    for (WaveformSample **p1 = ptrs; p1 < end - 1; p1++) {
        WaveformSample **min_idx = p1;
        for (WaveformSample **p2 = p1 + 1; p2 < end; p2++) {
            if (fabs(get_voltage(*p2, phase)) < fabs(get_voltage(*min_idx, phase))) {
                min_idx = p2;
            }
        }
        WaveformSample *temp = *min_idx;
        *min_idx = *p1;
        *p1 = temp;
    }
}