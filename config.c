#define _GNU_SOURCE
#include "global.h"
#include "config.h"

static string filename = "client.conf";

void
setFileName(string import) {
    printf("Using %s as config file\n", import);
    filename = import;
}

config
readConfig() {
    FILE *stream;
    string word;
    string sep = "\n\t =";
    config retval;
    size_t len = 0;
    int read;
    string line = NULL;

    memset(&retval, 0, sizeof(retval));
    
    stream = fopen(filename, "r");
    if (stream == NULL) {
        printf("Couldn't read %s\n", filename);
        printf("Trying again but with default filename (client.conf)\n");
        filename = "client.conf";
        stream = fopen(filename, "r");
        if (stream == NULL) {
            printf("ERROR: No config file found!\n");
            goto stop;
        }
    }
    
    while ((read = getline(&line, &len, stream)) > 0) {
        word = strtok(line, sep);
        if (strcmp(word, "Hostname") == 0) {
            retval.hostname = strdup(strtok(NULL, sep));
        }
        else if (strcmp(word, "Portnumber") == 0) {
            retval.portnumber = strdup(strtok(NULL, sep));
        }
        else if (strcmp(word, "Gamekindname") == 0) {
            retval.gamekind = strdup(strtok(NULL, sep));
        }
    }
    fclose(stream);
    
stop:
    free(line);
    return retval;
}

/*
 VERY IMPORTANT:
    To prevent memory leaks. Call this function after you are done using readConf()
*/
void
freeConfig(config structure) {
    free(structure.hostname);
    free(structure.portnumber);
    free(structure.gamekind);
}
