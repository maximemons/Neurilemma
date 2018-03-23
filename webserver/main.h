void traitement_signal(int sig);
void initialiser_signaux(void);
void badRequest(FILE *client, int code);
void goodRequest(FILE *client, const char* msg);
char *fgets_or_exit(char *buffer, int size, FILE *stream);
int parse_http_request(char *request_line, http_request *request);
void skip_headers(char *buffer, int size, FILE *stream);
void send_status(FILE *client, int code, char *reason_phrase);
void send_response(FILE *client, int valid_request, http_request parsed_request, int size, int fd, int socket_client, char *root_directory);
char *rewrite_target(char *target);
int check_and_open(const char *target, const char *document_root);
int get_file_size(int fd);
int copy(int in, int out);