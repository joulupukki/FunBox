// Hall Reverb implementation
// Copyright (C) 2025 Boyd Timothy

#include "hall_reverb.h"

namespace flick {

// Define the static constants
constexpr std::array<int, HallReverb::kNumCombFilters> HallReverb::kCombDelays;
constexpr std::array<int, HallReverb::kNumAllPassFilters> HallReverb::kAllPassDelays;

void HallReverb::Init(float sample_rate, size_t max_delay) {
    sample_rate_ = sample_rate;

    // Initialize comb filters
    for (auto& comb : comb_filters_) {
        comb.Init(sample_rate, lp_freq_);
        comb.feedback = feedback_;
        comb.damp = 0.5f; // Moderate damping
    }

    // Set comb filter delays
    for (size_t i = 0; i < kNumCombFilters; ++i) {
        comb_filters_[i].delay.SetDelay(static_cast<float>(kCombDelays[i]));
    }

    // Initialize all-pass filters
    for (auto& ap : allpass_filters_) {
        ap.Init();
        ap.feedback = 0.7f; // Standard all-pass feedback
    }

    // Set all-pass delays
    for (size_t i = 0; i < kNumAllPassFilters; ++i) {
        allpass_filters_[i].delay.SetDelay(static_cast<float>(kAllPassDelays[i]));
    }
}

void HallReverb::Process(const float* in_left, const float* in_right,
                         float* out_left, float* out_right, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        ProcessSample(in_left[i], in_right[i], &out_left[i], &out_right[i]);
    }
}

void HallReverb::ProcessSample(float in_left, float in_right, float* out_left, float* out_right) {
    // Mix left and right input for mono processing
    float mono_in = (in_left + in_right) * 0.5f;

    // Process through comb filters
    float comb_out = 0.0f;
    for (auto& comb : comb_filters_) {
        comb_out += comb.Process(mono_in);
    }
    comb_out /= kNumCombFilters; // Average the comb outputs

    // Process through all-pass filters in series
    float reverb_out = comb_out;
    for (auto& ap : allpass_filters_) {
        reverb_out = ap.Process(reverb_out);
    }

    // Apply dry/wet mix
    float dry = mono_in * (1.0f - dry_wet_);
    float wet = reverb_out * dry_wet_;

    // Output to both channels
    *out_left = dry + wet;
    *out_right = dry + wet;
}

} // namespace flick