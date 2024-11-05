#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>

struct Job {
    int jobId;
    int arrivalDay;
    int arrivalHour;
    int memReq;
    int cpuReq;
    int exeTime;
    int grossValue;

    Job(int id, int day, int hour, int mem, int cpu, int time)
        : jobId(id), arrivalDay(day), arrivalHour(hour), memReq(mem), cpuReq(cpu), exeTime(time) {
        grossValue = exeTime * cpuReq * memReq;
    }
};

struct WorkerNode {
    int cores = 24;
    int memory = 64;
    int availableCores;
    int availableMemory;

    WorkerNode() : availableCores(cores), availableMemory(memory) {}
};

class Scheduler {
    std::vector<Job> jobs;
    std::vector<WorkerNode> nodes;
    std::queue<Job> jobQueue;
    int policyType;

public:
    Scheduler(int policy) : policyType(policy) {
        nodes.resize(128); 
    }

    void loadJobs(const std::string &filename) {
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            int id, day, hour, mem, cpu, time;
            if (ss >> id >> day >> hour >> mem >> cpu >> time) {
                jobs.emplace_back(id, day, hour, mem, cpu, time);
            }
        }
    }

    void queueJobs(int policy) {
        switch (policy) {
            case 0: // FIFO
                for (const auto &job : jobs) {
                    jobQueue.push(job);
                }
                break;
            case 1: // Lowest gross value
                std::sort(jobs.begin(), jobs.end(), [](Job &a, Job &b) {
                    return a.grossValue < b.grossValue;
                });
                for (const auto &job : jobs) {
                    jobQueue.push(job);
                }
                break;
            case 2: // Shortest execution time
                std::sort(jobs.begin(), jobs.end(), [](Job &a, Job &b) {
                    return a.exeTime < b.exeTime;
                });
                for (const auto &job : jobs) {
                    jobQueue.push(job);
                }
                break;
            default:
                std::cerr << "Invalid policy type!" << std::endl;
        }
    }

    bool allocateJobToNode(Job &job, int allocationPolicy) {
        if (allocationPolicy == 0) { 
            for (auto &node : nodes) {
                if (node.availableCores >= job.cpuReq && node.availableMemory >= job.memReq) {
                    node.availableCores -= job.cpuReq;
                    node.availableMemory -= job.memReq;
                    return true;
                }
            }
        }
        return false;
    }

    void simulate() {
        while (!jobQueue.empty()) {
            Job job = jobQueue.front();
            jobQueue.pop();
            if (!allocateJobToNode(job, policyType)) {
                jobQueue.push(job); 
            }
        }
    }

    void exportUsageData(const std::string &filename) {
        std::ofstream file(filename);
        file << "Node,CPU_Usage,Memory_Usage\n";
        int nodeIndex = 1;
        for (const auto &node : nodes) {
            int cpuUsage = 24 - node.availableCores;
            int memoryUsage = 64 - node.availableMemory;
            file << "Node " << nodeIndex++ << "," << cpuUsage << "," << memoryUsage << "\n";
        }
    }
};

int main() {
    Scheduler scheduler(0);
    scheduler.loadJobs("JobArrival.txt"); 
    scheduler.queueJobs(0);
    scheduler.simulate(); 
    scheduler.exportUsageData("usage_data.csv"); 

    std::cout << "Simulation complete. Resource data exported to usage_data.csv." << std::endl;
    return 0;
}
