#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "udpServer.h"
#include "sampler.h"
#include "dip.h"

#define UDP_PORT 12345
#define MAX_PACKET_SIZE 1500
#define SIZE_OF_ONE_SAMPLE 16
#define SAMPLES_PER_LINE 10

static int s_socket_fd = -1;
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_thread;
static bool s_stop_flag = false;
static char s_lastCommand[MAX_PACKET_SIZE];

static bool isStopping(void) {
    pthread_mutex_lock(&s_lock);
    bool stopping = s_stop_flag;
    pthread_mutex_unlock(&s_lock);
    return stopping;
}

static void sendReply(const char *reply, const struct sockaddr_in *sender_address, socklen_t sender_addrlen) {
    ssize_t sent = sendto(s_socket_fd, reply, strlen(reply), 0, (const struct sockaddr *)sender_address, sender_addrlen);
    if (sent == -1) {
        perror("Error sending packets");
    }
}

static void sendHistory(const struct sockaddr_in *sender_address, socklen_t sender_addrlen) {
    int size;
    double *history = Sampler_getHistory(&size);
    if(size == 0) {
        sendReply("No samples yet.\n", sender_address, sender_addrlen);
        free(history);
        return;
    }

    char packet[MAX_PACKET_SIZE];
    int packet_size = 0;
    packet[0] = '\0';

    for (int i = 0; i < size; i++) {
        char sampleText[SIZE_OF_ONE_SAMPLE];

        const char *ending;
        if ((i + 1) % SAMPLES_PER_LINE == 0) {
            ending = ",\n";
        } else {
            ending = ", ";
        }

        int textLength = snprintf(sampleText, sizeof(sampleText), "%.3f%s", history[i], ending);

        if ((packet_size + textLength + 1) > MAX_PACKET_SIZE) {
            sendReply(packet, sender_address, sender_addrlen);
            packet_size = 0;
            packet[0] = '\0';
        }

        strcpy(packet + packet_size, sampleText);
        packet_size += textLength;
    }

    if (packet_size > 0) {
        sendReply(packet, sender_address, sender_addrlen);
    }

    free(history);
}

static void handleCommand(const char *command, const struct sockaddr_in *sender_address, socklen_t sender_addrlen) {
    char reply[MAX_PACKET_SIZE];
    if (strcmp (command, "count") == 0) {
        snprintf(reply, sizeof(reply), "# samples taken total: %lld\n", Sampler_getTotalSamples());
    } else if (strcmp (command, "length") == 0) {
        snprintf(reply, sizeof(reply), "# samples taken last second: %d\n", Sampler_getHistorySize());
    } else if (strcmp (command, "dips") == 0) {
        snprintf(reply, sizeof(reply), "# Dips: %d\n", Dip_getLastCount());
    } else if (strcmp (command, "history") == 0) {
        sendHistory(sender_address, sender_addrlen);
        return;
    } else if (strcmp (command, "help") == 0 || (strcmp (command, "?") == 0)) {
        snprintf(reply, sizeof(reply), "Accepted command examples:\n"
                "count   -- get the total number of samples taken.\n"
                "length  -- get the number of samples taken in the previously completed second.\n"
                "dips    -- get the number of dips in the previously completed second.\n"
                "history -- get all the samples in the previously completed second.\n"
                "stop    -- cause the server program to end.\n"
                "<enter> -- repeat last command.\n");
    } else if (strcmp (command, "stop") == 0) {
        snprintf(reply, sizeof(reply), "Program terminating.\n");

        pthread_mutex_lock(&s_lock);
        s_stop_flag = true;
        pthread_mutex_unlock(&s_lock);

    } else {
        snprintf(reply, sizeof(reply), "Unknown command. Type 'help' for the list of commands.\n");
    }

    sendReply(reply, sender_address, sender_addrlen);
}

static void *udpServerThread (void *arg) {
    (void)arg;

    while(!isStopping()) {

        struct sockaddr_in sender_address = {0};
        socklen_t sender_addrlen = sizeof(sender_address);

        char buffer[MAX_PACKET_SIZE];
        ssize_t received = recvfrom(s_socket_fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender_address, &sender_addrlen);
        if (received == -1) {
            perror("Error receiving packet");
            continue;
        }

        buffer[received] = '\0';

        if (received > 0 && buffer[received - 1] == '\n') {
            buffer[received - 1] = '\0';
        }

        if (buffer[0] == '\0') {
            strcpy(buffer, s_lastCommand);
        } else {
            strcpy(s_lastCommand, buffer);
        }

        handleCommand(buffer, &sender_address, sender_addrlen);

    }
    return NULL;
}

bool UdpServer_init(void) {
    s_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_socket_fd < 0) {
        perror("Error creating socket");
        return false;
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET; // IPv4
    address.sin_port = htons(UDP_PORT); // listening port
    address.sin_addr.s_addr = htonl(INADDR_ANY); //accept any of this board's addresses

    if (bind(s_socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error binding socket");
        close(s_socket_fd);
        s_socket_fd = -1;
        return false;
    }

    if (pthread_create(&s_thread, NULL, udpServerThread, NULL) != 0) {
        fprintf(stderr, "pthread_create failed\n");
        close(s_socket_fd);
        s_socket_fd = -1;
        return false;
    }

    return true;
}

bool UdpServer_isStopRequested(void) {
    return isStopping();
}

void UdpServer_cleanup(void) {
    pthread_join(s_thread, NULL);
    close(s_socket_fd);
    s_socket_fd = -1;
}