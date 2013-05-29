#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <pcre.h> 

#include "statsd-client.c"

#define DEFAULT_PORT 8125
#define DEFAULT_HOST  "127.0.0.1"
#define DEFAULT_SAMPLE_RATE 1.0

#define VERSION "0.5"


void showHelp() {
    printf("log-lines-counter version %s\n", VERSION);
    printf("emits a hit to the statsd server for each line received\n\n");
    printf("Usage: cat log | log-lines-counter [OPTIONS]\n");
    printf("\t-N <metric>         metric name to update (required)\n");
    printf("\t-h <host>           statsd host (default: %s)\n", DEFAULT_HOST);
    printf("\t-p <post>           statsd port (default: %d)\n", DEFAULT_PORT);
    printf("\t-s <rate>           sample rate (default: %2.2f)\n", DEFAULT_SAMPLE_RATE);
    printf("\t-d                  debug mode\n");
    printf("\t-e <pattern>        regex pattern to use\n");
    printf("\t-i                  ignore case distinctions in pattern\n");
    printf("\t-v                  invert match: select non-matching lines for pattern\n");

}

char *trim(char *s) {
    char *ptr;
    if (!s)
        return NULL;   // handle NULL string
    if (!*s)
        return s;      // handle empty string
    for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace(*ptr); --ptr);
    ptr[1] = '\0';
    return s;
}

int main(int argc, char *argv[])
{


    char *metric_name = NULL;
    char *pattern = NULL;
    int c;

    float sample_rate = DEFAULT_SAMPLE_RATE;
    int port = DEFAULT_PORT;
    char *host = DEFAULT_HOST;
    int verbose = 0;
    int nonmatch = 0;

    const char *pcreErrorStr;
    int pcreErrorOffset;
    pcre *reCompiled;
    pcre_extra *pcreExtra;
    int subStrVec[30];
    int pcreExecRet;
    int pcreFlags = 0;


    while ((c = getopt (argc, argv, "e:dvih::p::s:N:")) != -1) {
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
            case 'e':
                pattern = malloc(strlen(optarg));
                strcpy(pattern, optarg);
            break;
            case 's':
                sample_rate = (float) atof(optarg);
            break;
            case 'd':
                verbose = 1;
            break;
            case 'v':
                nonmatch = 1;
            break;             
            case 'i':
                pcreFlags = pcreFlags | PCRE_CASELESS;
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
    int send = 0;

    statsd_init(host, port);

    // setup the regex if we have a pattern
    if (pattern != NULL) {

        if (verbose) {
          syslog(LOG_INFO, "line must%s match '%s'", (nonmatch == 1 ? " NOT" : ""), pattern);
        }

        reCompiled = pcre_compile(pattern, pcreFlags, &pcreErrorStr, &pcreErrorOffset, NULL);

        if(reCompiled == NULL) {
            printf("ERROR: Could not compile regex '%s': %s\n", pattern, pcreErrorStr);
            return 1;
        }

        pcreExtra = pcre_study(reCompiled, 0, &pcreErrorStr);

        if(pcreErrorStr != NULL) {
            printf("ERROR: Could not optimize regex '%s': %s\n", pattern, pcreErrorStr);
            return 1;
        }
    }

    while (getline(&line, &size, stdin) != -1) {

        line = trim(line);
        size = strlen(line);

        send = 0, pcreExecRet = 0;
        if (pattern != NULL) {

            pcreExecRet = pcre_exec(reCompiled, pcreExtra, line, size, 0, 0, subStrVec, 30);


            if((pcreExecRet == PCRE_ERROR_NOMATCH) && nonmatch) {
                send = 1; /* send the hit */
            }
            else if ((pcreExecRet > 0) && !nonmatch) {
                send = 1; /* send the hit */
            }
        }
        else {
            send = 1; /* send the hit */
        }

        if (send) {
            if (verbose) {
                syslog(LOG_INFO, "sending hit to statsd @ sample rate: %2.2f", sample_rate);
            }

            statsd_count(metric_name, 1, sample_rate);

        }


    }

    if (verbose) {
        syslog(LOG_INFO, "exiting...");
    }

    if (pattern != NULL) {
        // Free up the regular expression.
        pcre_free(reCompiled);

        // Free up the EXTRA PCRE value (may be NULL at this point)
        if(pcreExtra != NULL)
            pcre_free(pcreExtra);
    }
    
    statsd_finalize();
    return 0;
}

