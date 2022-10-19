#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// creates JSON credentials payload
char *create_json_credentials(char *username, char *password);

// creates JSON book information payload
char *create_json_book_info(char *title, char *author, char *genre,
                            int page_count, char *publisher);

// parses JSON with JWT token
char *extract_jwt_token_response(char *response);

// constructs book url from access route and book ID
char *construct_book_url(char *access_route, int book_id);

#endif
