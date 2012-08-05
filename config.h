typedef struct {
    int node_id;
    char *interface;
    int port;
    int queue_size;
    int master_port;
    char * master_host;
} Config;

Config *Config_create();

extern Config *config;
