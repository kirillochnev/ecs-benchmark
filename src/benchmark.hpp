#pragma once

#include <cmath>
#include <cstdint>
#include <chrono>
#include <ostream>
#include <numeric>
#include <algorithm>
#include <iostream>

namespace bench {
    enum class ReportType : uint32_t {
        kAvrOnly,
        kShort,
        kDefault,
        kFull,
    };

    class BenchMark {
    public:
        template<typename _F, typename _BeforeRun>
        void run(_F&& function, uint32_t iterations = 1, _BeforeRun&& before_run = [] {}) {
            metrics_.reserve(metrics_.size() + iterations);
            for (uint32_t i = 0; i < iterations; ++i) {
                before_run();
                const auto begin = Clock::now();
                function();
                const auto end = Clock::now();
                metrics_.push_back(getMetricItems(end - begin));
            }
        }

        void printMetrics(std::ostream& out) const noexcept {
            for (const auto value: metrics_) {
                out << value << " ";
            }
            out.flush();
        }

        void reset() noexcept {
            metrics_.clear();
        }

        void setEntityCount(uint32_t count) noexcept {
            entities_count_ = count;
        }

        double average() const noexcept {
            const auto sum = std::accumulate(metrics_.begin(), metrics_.end(), 0.0);
            return sum / static_cast<double>(metrics_.size());
        }

        void show(std::ostream& out, ReportType report_type = ReportType::kDefault) noexcept {
            if (metrics_.empty()) {
                return;
            }
            const auto avr = average();
            if (report_type == ReportType::kAvrOnly) {
                out << avr << std::endl;
                return;
            }
            const std::string unit = entities_count_ < 1 ? "ms" : "ops";
            if (metrics_.size() < 2) {
                if (report_type == ReportType::kShort) {
                    out << metrics_.front() << std::endl;
                } else {
                    out << "Result: " << std::to_string(metrics_.front()) << unit << std::endl;
                }
                return;
            }

            std::sort(metrics_.begin(), metrics_.end());
            auto med = metrics_[metrics_.size() / 2];
            if (metrics_.size() % 2 == 0) {
                med = (med + metrics_[metrics_.size() / 2 - 1]) * 0.5;
            }
            double variance = 0.0;
            for (auto x: metrics_) {
                variance += (avr - x) * (avr - x) / static_cast<double>(metrics_.size());
            }

            if (report_type == ReportType::kShort) {
                out << avr << " " << med << " " << metrics_.front() << " " << metrics_.back() << std::endl;
            }

            if (report_type == ReportType::kDefault || report_type == ReportType::kFull) {
                out << "Call count: " << metrics_.size() << ", Arv: " << toStr(avr) << ", med: " << toStr(med)
                    << ", min: " << toStr(metrics_.front()) << ", max: "
                    << toStr(metrics_.back()) << ", variance: " << std::to_string(variance) << ", sigma: "
                    << std::to_string(sqrt(variance)) << std::endl;

                if (report_type == ReportType::kFull) {
                    printMetrics(out);
                }
            }

        }

    private:

        std::string toStr(double value) const noexcept {
            if (entities_count_ < 1) {
                return std::to_string(value) + "ms";
            }
            return std::to_string(static_cast<uint64_t >(value)) + "ops";
        }

        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;

        template<typename T>
        static double getSeconds(const T& pDur) noexcept {
            constexpr double Microseconds2SecondConvertK = 0.000001;
            const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(pDur).count();
            return static_cast<double>(microseconds) * Microseconds2SecondConvertK;
        }

        template<typename T>
        double getMetricItems(const T& dur) const noexcept {
            const auto dt = getSeconds(dur);
            if (entities_count_ < 1) {
                return dt * 1000.0; // ms
            }
            return entities_count_ / dt; // ops
        }

        uint32_t entities_count_ = 0;
        std::vector<double> metrics_;

    };

    struct Config {
        struct TestInfo {
            uint32_t iterations;
            std::ostream* output = nullptr;
            ReportType report_type = ReportType::kDefault;
            std::ostream& getOutStream() const noexcept {
                return output != nullptr ? *output : std::cout;
            }
        };
        uint32_t entity_count = 1000000;
        TestInfo create_world{10};
        TestInfo update_world{100};
        bool create_extra = false;
        bool remove_half = false;
        bool parallel_update = false;
    };
}
