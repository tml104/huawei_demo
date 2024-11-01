#pragma once
#include <ctime>
#include <vector>

#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif

namespace Timer {
	class Timer {
	public:
		Timer() { Start(); }

		void Start() {
			split_vec.clear();
			start_time = std::clock();
		}

		void Restart() {
			Start();
		}

		double ElapsedSeconds() const {
			return Elapsed() / static_cast<double>(CLOCKS_PER_SEC);
		}

		double GetSeconds(std::clock_t c) const {
			return (c - start_time) / static_cast<double>(CLOCKS_PER_SEC);
		}

		void Split(std::string split_name) {
			split_vec.push_back(
				std::make_pair( split_name, std::clock() )
			);
		}

		void PrintTimes() {
			LOG_INFO("===== Timer =====");

			for (int i = 0; i < split_vec.size(); i++) {
				auto p = split_vec[i];
				LOG_INFO("%s: \t%.3lf", p.first.c_str(), GetSeconds(p.second));
			}
			double now_sec = ElapsedSeconds();
			LOG_INFO("Total: \t%.3lf", now_sec);

			LOG_INFO("===== Timer End =====");
		}

	private:
		std::clock_t start_time;

		std::vector<std::pair<std::string, std::clock_t>> split_vec;

		std::clock_t Elapsed() const {
			return std::clock() - start_time;
		}
	};
}

