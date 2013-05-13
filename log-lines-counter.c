#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "statsd-client.c"

#define DEFAULT_PORT 8125
#define DEFAULT_HOST  "127.0.0.1"
#define DEFAULT_SAMPLE_RATE 1.0

#define VERSION "0.3"


void showHelp() {
    printf("log-lines-counter version %s\n", VERSION);
    printf("emits a hit to the statsd server for each line received\n\n");
    printf("Usage: cat log | log-lines-counter [OPTIONS]\n");
    printf("\t-N <metric>         metric name to update (required)\n");
    printf("\t-h <host>           statsd host (default: %s)\n", DEFAULT_HOST);
    printf("\t-p <post>           statsd port (default: %d)\n", DEFAULT_PORT);
    printf("\t-s <rate>           sample rate (default: %2.2f)\n", DEFAULT_SAMPLE_RATE);
    printf("\t-v                  verbose mode\n");

}

int main(int argc, char *argv[])
{


    char *metric_name = NULL;
    int c;

    float sample_rate = DEFAULT_SAMPLE_RATE;
    int port = DEFAULT_PORT;
    char *host = DEFAULT_HOST;
    int verbose = 0;


    while ((c = getopt (argc, argv, "vh::p::s:N:")) != -1) {
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
            case 'v':
                verbose = 1;
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

    if (verbose) {
        setlogmask(LOG_UPTO(LOG_INFO));
        openlog("log-lines-counter",  LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
        syslog(LOG_INFO, "statsd hosts: %s:%d", host, port);
        syslog(LOG_INFO, "metric name: %s", metric_name);
        syslog(LOG_INFO, "sample rate: %2.2f", sample_rate);
    }

    char *line = NULL;
    size_t size;

    statsd_init(host, port);

    while (getline(&line, &size, stdin) != -1) {
        statsd_count(metric_name, 1, sample_rate);

        if (verbose) {
            syslog(LOG_INFO, "sent hit to statsd @ sample rate: %2.2f", sample_rate);
        }
    }

    if (verbose) {
        syslog(LOG_INFO, "exiting...");
    }
    
    statsd_finalize();
    return 0;
}

