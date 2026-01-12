#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <limits>

struct LogEntry {
    long long timestamp;
    std::string request_id;
    int latency;
};

class LogAnalyzer {
private:
    std::unordered_map<std::string, std::vector<LogEntry>> logs_by_service;
    std::unordered_set<std::string> seen_request_ids;

    bool parseLine(const std::string& line,
                   long long& timestamp,
                   std::string& service,
                   std::string& request_id,
                   int& latency) const
    {
        std::stringstream ss(line);
        std::string token;

        std::vector<std::string> parts;
        while (std::getline(ss, token, '|')) {
            parts.push_back(token);
        }

        if (parts.size() != 5)
            return false;

        try {
            timestamp = std::stoll(parts[0]);
            latency   = std::stoi(parts[4]);
        } catch (...) {
            return false;
        }

        service = parts[1];
        request_id = parts[3];
        return true;
    }

public:
    void ingestFile(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line)) {
            long long ts;
            std::string service, req_id;
            int latency;

            if (!parseLine(line, ts, service, req_id, latency))
                continue;

            if (seen_request_ids.count(req_id))
                continue;

            seen_request_ids.insert(req_id);
            logs_by_service[service].push_back({ts, req_id, latency});
        }

        // Sort logs per service by timestamp
        for (auto& [service, logs] : logs_by_service) {
            std::sort(logs.begin(), logs.end(),
                      [](const LogEntry& a, const LogEntry& b) {
                          return a.timestamp < b.timestamp;
                      });
        }
    }

    void computeAnalytics() const {
        for (const auto& [service, logs] : logs_by_service) {
            if (logs.empty()) continue;

            int min_latency = std::numeric_limits<int>::max();
            int max_latency = std::numeric_limits<int>::min();
            long long sum_latency = 0;
            long long max_idle_gap = 0;

            for (size_t i = 0; i < logs.size(); ++i) {
                min_latency = std::min(min_latency, logs[i].latency);
                max_latency = std::max(max_latency, logs[i].latency);
                sum_latency += logs[i].latency;

                if (i > 0) {
                    long long gap = logs[i].timestamp - logs[i - 1].timestamp;
                    max_idle_gap = std::max(max_idle_gap, gap);
                }
            }

            double avg_latency =
                static_cast<double>(sum_latency) / logs.size();

            std::cout << "Service: " << service << "\n";
            std::cout << "Requests: " << logs.size() << "\n";
            std::cout << "Min Latency(ms): " << min_latency << "\n";
            std::cout << "Max Latency(ms): " << max_latency << "\n";
            std::cout << "Avg Latency(ms): " << avg_latency << "\n";
            std::cout << "Max Idle Gap(ms): " << max_idle_gap << "\n\n";
        }
    }

    std::vector<LogEntry> query(const std::string& service,
                               long long start_ts,
                               long long end_ts) const
    {
        std::vector<LogEntry> result;

        auto it = logs_by_service.find(service);
        if (it == logs_by_service.end())
            return result;

        const auto& logs = it->second;

        auto start_it = std::lower_bound(
            logs.begin(), logs.end(), start_ts,
            [](const LogEntry& log, long long ts) {
                return log.timestamp < ts;
            });

        auto end_it = std::upper_bound(
            logs.begin(), logs.end(), end_ts,
            [](long long ts, const LogEntry& log) {
                return ts < log.timestamp;
            });

        result.insert(result.end(), start_it, end_it);
        return result;
    }
};

int main() {
    LogAnalyzer analyzer;
    analyzer.ingestFile("logs.txt");
    analyzer.computeAnalytics();

    // Example query
    auto results = analyzer.query("auth", 1700000010000, 1700000013000);
    std::cout << "Query Results: " << results.size() << " entries\n";
}
