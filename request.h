#ifndef _REQUESTS_
#define _REQUESTS_
#include "parson.h"

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
							char **cookies, int cookies_count, char* authorization);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type,  JSON_Value * json,
							char** cookies, int cookies_count, char* authorization);
				
char *compute_delete_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, char *authorization);

#endif
