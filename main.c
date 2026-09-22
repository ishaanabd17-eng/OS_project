#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int tick;
    int altitude;
    int airspeed;
    int fuel;
    int engine_on;
} Telemetry;

static volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int sig) {
    (void)sig;
    stop_requested = 1;
}

static void install_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static void simulator_process(int write_fd) {
    close(STDIN_FILENO);
    srand((unsigned int)(time(NULL) ^ getpid()));

    for (int tick = 1; tick <= 20 && !stop_requested; ++tick) {
        Telemetry t;
        t.tick = tick;
        t.altitude = 10000 + (rand() % 1201) - 600;
        t.airspeed = 450 + (rand() % 81) - 40;
        t.fuel = 100 - tick * 4;
        if (t.fuel < 0) t.fuel = 0;
        t.engine_on = (t.fuel > 0);

        if (write(write_fd, &t, sizeof(t)) != (ssize_t)sizeof(t)) {
            break;
        }

        sleep(1);
    }

    close(write_fd);
    _exit(EXIT_SUCCESS);
}

static void monitor_process(int read_fd, int log_fd) {
    Telemetry t;
    ssize_t n;

    dprintf(STDOUT_FILENO,
            "\n=== FLIGHT MONITOR (PID %ld, PPID %ld) ===\n",
            (long)getpid(), (long)getppid());

    while ((n = read(read_fd, &t, sizeof(t))) > 0) {
        if (n != (ssize_t)sizeof(t))
            continue;

        const char *status = "NORMAL";
        if (t.fuel <= 20)
            status = "LOW FUEL";
        if (t.altitude < 9500 || t.altitude > 10500)
            status = "ALTITUDE WARNING";

        dprintf(STDOUT_FILENO,
                "[TICK %02d] ALT=%5d ft | SPEED=%3d km/h | FUEL=%3d%% | ENGINE=%s | %s\n",
                t.tick, t.altitude, t.airspeed, t.fuel,
                t.engine_on ? "ON " : "OFF", status);

        dprintf(log_fd,
                "tick=%d altitude=%d airspeed=%d fuel=%d engine=%s status=%s\n",
                t.tick, t.altitude, t.airspeed, t.fuel,
                t.engine_on ? "ON" : "OFF", status);
    }

    close(read_fd);
    close(log_fd);
    _exit(EXIT_SUCCESS);
}

int main(void) {
    int pipefd[2];

    install_handlers();

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    int log_fd = open("logs/flight.log",
                      O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("open logs/flight.log");
        close(pipefd[0]);
        close(pipefd[1]);
        return EXIT_FAILURE;
    }

    printf("=============================================\n");
    printf(" Flight Simulator and Monitoring System\n");
    printf(" Team 3 | Section 9\n");
    printf(" Faizaan (2520030359) | Ishaan (2520030267)\n");
    printf("=============================================\n");
    printf("Main controller PID: %ld\n", (long)getpid());
    printf("Creating simulator and monitor processes...\n");

    pid_t simulator_pid = fork();
    if (simulator_pid == -1) {
        perror("fork simulator");
        close(pipefd[0]); close(pipefd[1]); close(log_fd);
        return EXIT_FAILURE;
    }

    if (simulator_pid == 0) {
        close(pipefd[0]);
        close(log_fd);
        simulator_process(pipefd[1]);
    }

    pid_t monitor_pid = fork();
    if (monitor_pid == -1) {
        perror("fork monitor");
        kill(simulator_pid, SIGTERM);
        close(pipefd[0]); close(pipefd[1]); close(log_fd);
        waitpid(simulator_pid, NULL, 0);
        return EXIT_FAILURE;
    }

    if (monitor_pid == 0) {
        close(pipefd[1]);
        monitor_process(pipefd[0], log_fd);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    close(log_fd);

    printf("Simulator PID: %ld\n", (long)simulator_pid);
    printf("Monitor PID:   %ld\n", (long)monitor_pid);
    printf("IPC: anonymous pipe() | Signals: SIGINT/SIGTERM\n");
    printf("Logging: logs/flight.log\n\n");

    if (stop_requested) {
        kill(simulator_pid, SIGTERM);
        kill(monitor_pid, SIGTERM);
    }

    int status;
    waitpid(simulator_pid, &status, 0);
    waitpid(monitor_pid, &status, 0);

    printf("\nAll child processes terminated safely.\n");
    printf("Parent PID %ld exiting.\n", (long)getpid());

    return EXIT_SUCCESS;
}
