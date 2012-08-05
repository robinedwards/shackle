#include "config.h"
#include "die.h"
#include <stdlib.h>

Config *Config_create() {
    Config *config = malloc(sizeof(Config));

    char *node_id, *interface, *port, *queue_size, *master_port, *master_host;

    node_id = getenv("SHACKLE_NODE_ID");
    if (node_id == NULL)
        die("No node_id set, please set SHACKLE_NODE_ID");
    config->node_id = atoi(node_id);

    interface = getenv("SHACKLE_INTERFACE");
    if (interface == NULL)
        config->interface = "0.0.0.0";
    else
        config->interface = interface;

    port = getenv("SHACKLE_PORT");
    if (port == NULL)
        config->port = 9999;
    else
        config->port = atoi(port);

    master_port = getenv("SHACKLE_MASTER_PORT");
    if (master_port == NULL)
        die("No master port set, please set SHACKLE_MASTER_PORT");
    config->master_port = atoi(master_port);

    master_host = getenv("SHACKLE_MASTER_HOST");
    if (master_host == NULL)
        die("No master host set, please set SHACKLE_MASTER_HOST");
    config->master_host = master_host;

    queue_size = getenv("SHACKLE_QUEUE_SIZE");
    if (queue_size == NULL)
        config->queue_size = 4;
    else
        config->queue_size = atoi(queue_size);

    return config;
}
