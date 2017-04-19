/***********
File: multilookup.h
***********/
#define MULTILOOKUP_H

#define NUM_THREADS 2
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define SBUFSIZE 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN //from arpa/inet.h
#define MAX_DNS_LOOKUPS 1000

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
//#define SBUFSIZE 1025
#define INPUTFS "%1024s"

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>