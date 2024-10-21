#include <curl/curl.h>

#define MAX_DATA_BYTES    16384
#define MAX_REQUESTS        256 // Must be 1 or more

#define AQ2WORLD_STAT_URL "https://apigateway.aq2world.com"

typedef struct request_s {
    char* url;
    char* payload;
    char *user;
    CURL* handle;
    size_t data_count;
    char data[MAX_DATA_BYTES];
} request_t;

typedef struct request_list_s {
    struct request_list_s *next;
    request_t request;
} request_list_t;

extern request_list_t *active_requests, *unused_requests;
extern request_list_t request_nodes[MAX_REQUESTS];
extern CURLM *stack;
extern size_t current_requests;

void lc_server_announce(char *path, char *message);
void announce_server_populating(void);
qboolean message_timer_check(int delay);