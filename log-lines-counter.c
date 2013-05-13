#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/statsd-client.c"

#define DEFAULT_PORT 8125
#define DEFAULT_HOST  "127.0.0.1"
#define DEFAULT_SAMPLE_RATE 1.0

#define VERSION "0.2"


void showHelp() {
    printf("log-lines-counter version %s\n", VERSION);
    printf("emits a hit to the statsd server for each line received\n\n");
    printf("Usage: cat log | log-lines-counter [OPTIONS]\n");
    printf("-N <metric>\tmetric name to update (required)\n");
    printf("-h <host>\tstatsd host (default: %s)\n", DEFAULT_HOST);
    printf("-p <post>\tstatsd port (default: %d)\n", DEFAULT_PORT);
    printf("-s <rate>\tsample rate (default: %2.2f)\n", DEFAULT_SAMPLE_RATE);

}

int main(int argc, char *argv[])
{


    char *metric_name = NULL;
    int c;

    float sample_rate = DEFAULT_SAMPLE_RATE;
    int port = DEFAULT_PORT;
    char *host = DEFAULT_HOST;



    while ((c = getopt (argc, argv, "h::p::s:N:")) != -1) {
        switch (c) {
            case 'h':
                host = optarg;
            break;
            case 'p':
                port = atoi(optarg);
            break;
            case 'N':
                metric_name = malloc(strlen(optarg));
                strcpy(metric_name, optarg);
            break;
            case 's':
                sample_rate = (float) atof(optarg);
            break;
            case '?':
                showHelp();
                return 0;
            break;
            default:
                showHelp();
                return 1;
            break;
        }
    }

    if (metric_name == NULL) {
        fprintf(stderr, "%s: missing -N option\n", argv[0]);
        showHelp();
        return 1;
    }

    char *line = NULL;
    size_t size;

    statsd_init(host, port);

    while (getline(&line, &size, stdin) != -1) {
        statsd_count(metric_name, 1, sample_rate);
    }

    statsd_finalize();
    return 0;
}

