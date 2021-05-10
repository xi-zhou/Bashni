/*	config.h */

typedef char *string;

typedef struct {
    string hostname;
    string portnumber;
    string gamekind;
}config;

void setFileName(string import);

config readConfig();

void freeConfig(config structure);
