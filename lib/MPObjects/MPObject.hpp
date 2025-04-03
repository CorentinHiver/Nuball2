#ifndef MPOBJECT_HPP
#define MPOBJECT_HPP

#include "../libCo.hpp"

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

class MPObject {
  public:
  MPObject() = default;
  ~MPObject() {
    for (pid_t pid : m_processes) {
    kill(pid, SIGTERM); // Terminate any remaining processes
    }
  }

void startProcess(const std::string &command) {
  pid_t pid = fork();
  if (pid == 0) {
  // Child process
    execl("/bin/sh", "sh", "-c", command.c_str(), (char *)nullptr);
    _exit(EXIT_FAILURE); // If execl fails
  } else if (pid > 0) {
    // Parent process
    m_processes.push_back(pid);
    std::cout << "Started process with PID: " << pid << std::endl;
  } else {
    // Fork failed
    std::cerr << "Failed to start process: " << command << std::endl;
  }
}

void monitorProcesses() {
  while (!m_processes.empty()) {
    int status;
    pid_t pid = waitpid(-1, &status, 0);
    if (pid > 0) {
      std::cout << "Process " << pid << " exited with status " << status << std::endl;
      m_processes.erase(std::remove(m_processes.begin(), m_processes.end(), pid), m_processes.end());
    } else {
      std::cerr << "Failed to wait for process" << std::endl;
    }
  }
}

void stopAllProcesses() {
  for (pid_t pid : m_processes) {
    if (kill(pid, SIGTERM) == 0) {
      std::cout << "Sent SIGTERM to process " << pid << std::endl;
    } else {
      std::cerr << "Failed to send SIGTERM to process " << pid << std::endl;
    }
  }
}

private:
  std::vector<pid_t> m_processes;
};

#endif //MPOBJECT_HPP