#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define EMPTY ""

int main(int argc, char *argv[])
{
    int sockfd;
    
    // stores cookie and the JWT token for the current user
    // if no user is logged in then they are set to EMPTY
    char *cookie = EMPTY;
    char *JWT_token = EMPTY;

    // stores the current command given by user
    char *cmd = (char *) malloc(100);

    // loop that ends when the user enters "exit"
    while (1)
    {
        scanf("%99s", cmd);
        if (strcmp(cmd, "exit") == 0)
        {
            free(cmd);
            break;
        }

        else if (strcmp(cmd, "register") == 0)
        {
            char *message;
            char *response;
            char *username = (char*) malloc(100);
            char *password = (char*) malloc(100);

            printf("username=");
            scanf("%99s", username);
            printf("password=");
            scanf("%99s", password);

            // setting form_data to be sent to server in JSON form
            char **form_data = (char**) calloc(1, sizeof(char *));
            form_data[0] = create_json_credentials(username, password);

            message = compute_post_request("localhost", "/api/v1/tema/auth/register", "application/json", form_data, 1, NULL, 0, NULL);
            
            // opening connection
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // showing the status of the request to the user
            if (strstr(response, "is taken"))
                puts("Username already registered!\n");
            else if (strstr(response, "201 Created") || strstr(response, "200 OK"))
                puts("User created successfully\n");

            close(sockfd);
            free(username);
            free(password);
            free(form_data[0]);
            free(form_data);
            free(message);
            free(response);
        }

        else if (strcmp(cmd, "login") == 0)
        {
            char *message;
            char *response;
            char *username = (char*) malloc(100);
            char *password = (char*) malloc(100);

            printf("username=");
            scanf("%99s", username);
            printf("password=");
            scanf("%99s", password);

            // setting form_data to be sent to server in JSON form
            char **form_data = (char**) calloc(1, sizeof(char *));
            form_data[0] = create_json_credentials(username, password);

            message = compute_post_request("localhost", "/api/v1/tema/auth/login", "application/json", form_data, 1, NULL, 0, NULL);
            
            // opening connection
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // showing the status of the request to the user and setting the cookie variable
            // to the cookie received from the server 
            if (strstr(response, "error"))
                puts("Username or password incorrect!\n");
            else if (strstr(response, "200 OK"))
            {
                // extracting the cookie from the response
                char *response_cookie = strstr(response, "connect.sid=");
                response_cookie = strtok(response_cookie, ";");

                // setting the cookie variable to the one received from server
                cookie = malloc(strlen(response_cookie) + 1);
                strcpy(cookie, response_cookie);
                puts("Logged in successfully\n");
            }
            close(sockfd);
            free(username);
            free(password);
            free(form_data[0]);
            free(form_data);
            free(message);
            free(response);
        }

        else if (strcmp(cmd, "enter_library") == 0)
        {
            char *message;
            char *response;

            // adding current cookie to the cookies list to be sent to the server.
            // since we only intend to send one cookie, we are setting the list
            // to the cookie value
            char **cookies = &cookie;

            message = compute_get_request("localhost", "/api/v1/tema/library/access", NULL, cookies, 1, NULL);

            // opening connection
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // showing the status of the request to the user and setting the JWT_token variable
            // with the JWT token received from the server 
            if (strstr(response, "error"))
                puts("You are not logged in!\n");
            else if (strstr(response, "200 OK"))
            {
                JWT_token = extract_jwt_token_response(response);
                puts("Entered library\n");
            }
            
            close(sockfd);
            free(message);
            free(response);
        }

        else if(strcmp(cmd, "get_books") == 0)
        {
            char *message;
            char *response;

            message = compute_get_request("localhost", "/api/v1/tema/library/books", NULL, NULL, 0, JWT_token);

            // opening connection
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // showing the status of the request to the user and parsing the list of books
            // received from server. If the server returns an empty list, we print
            // "No books in library!", otherwise we print the list of books
            if (strstr(response, "error"))
                puts("You do not have access to the library!\n");
            else if (strstr(response, "200 OK"))
            {
                char *JSON_string = strstr(response, "[{");
                if (JSON_string == NULL)
                    puts("No books in library!\n");
                else
                {
                    JSON_Value *root_value = json_parse_string(JSON_string);
                    JSON_Array *books = json_value_get_array(root_value);

                    JSON_Object *book;
                    for (int i = 0; i < json_array_get_count(books); i++)
                    {
                        book = json_array_get_object(books, i);

                        int id = json_object_get_number(book, "id");
                        const char *title = json_object_get_string(book, "title");

                        printf("ID: %d\n", id);
                        printf("Titile: %s\n\n", title);

                        json_object_clear(book);
                    }
                    json_array_clear(books);
                    json_value_free(root_value);
                }
            }
            close(sockfd);
            free(message);
            free(response);
        }

        else if (strcmp(cmd, "get_book") == 0)
        {
            char *message;
            char *response;

            int id = 0;
            printf("id=");
            scanf("%d", &id);

            // id validation
            if (id < 0)
                puts("Please enter a non-negative number for ID\n");
            else if (id == 0)
                puts("Please enter a number for ID\n");
            else
            {   
                // constructing the url for the specific book
                char *url = construct_book_url("/api/v1/tema/library/books/", id);

                message = compute_get_request("localhost", url, NULL, NULL, 0, JWT_token);

                // opening connection
                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // showing the status of the request to the user and parsing the book information
                // received from server if a book with the given ID was found then
                // printing the book information
                if (strstr(response, "Error when decoding token"))
                    puts("You do not have access to the library!\n");
                else if (strstr(response, "No book was found!"))
                    puts("No book was found with the given id\n");
                else if (strstr(response, "200 OK"))
                {
                    JSON_Value *root_value = json_parse_string(strstr(response, "[{"));
                    JSON_Array *books = json_value_get_array(root_value);
                    JSON_Object *book = json_array_get_object(books, 0);

                    const char *title = json_object_get_string(book, "title");
                    const char *author = json_object_get_string(book, "author");
                    const char *publisher = json_object_get_string(book, "publisher");
                    const char *genre = json_object_get_string(book, "genre");
                    const int page_count = json_object_get_number(book, "page_count");

                    printf("Title: %s\n", title);
                    printf("Author: %s\n", author);
                    printf("Genre: %s\n", genre);
                    printf("Publisher: %s\n", publisher);
                    printf("Page count: %d\n\n", page_count);
                    json_object_clear(book);
                    json_array_clear(books);
                    json_value_free(root_value);
                }
                free(url);
                close(sockfd);
                free(message);
                free(response);
            }
        }

        else if (strcmp(cmd, "add_book") == 0)
        {
            char *message;
            char *response;

            char *title = (char *) malloc(100);
            char *author = (char *) malloc(100);
            char *genre = (char *) malloc(100);
            char *publisher = (char *) malloc(100);
            int page_count = 0;

            // read book info
            printf("title=");
            scanf("%99s", title);
            printf("author=");
            scanf("%99s", author);
            printf("genre=");
            scanf("%99s", genre);
            printf("publisher=");
            scanf("%99s", publisher);
            printf("page_count=");
            scanf("%d", &page_count);
            
            // checking if input is valid
            if (!(strlen(title) && strlen(author) && strlen(genre) && strlen(publisher)))
                puts("Please enter valid strings. Book not added\n");
            else if (page_count <= 0)
                puts("Page count must be a number bigger than 0. Book not added\n");
            else
            {
                // setting form_data to be sent to server in JSON form
                char **form_data = (char**) calloc(1, sizeof(char *));
                form_data[0] = create_json_book_info(title, author, genre, page_count, publisher);

                message = compute_post_request("localhost", "/api/v1/tema/library/books", "application/json", form_data, 1, NULL, 0, JWT_token);

                // opening connection
                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // showing the status of the request to the user
                if (strstr(response, "error"))
                    puts("You do not have access to the library!\n");
                else if (strstr(response, "200 OK"))
                    puts("Book added successfully\n");
                
                close(sockfd);
                free(title);
                free(author);
                free(genre);
                free(publisher);
                free(form_data[0]);
                free(form_data);
                free(message);
                free(response);
            }
        }

        else if (strcmp(cmd, "delete_book") == 0)
        {
            char *message;
            char *response;

            int id = 0;
            printf("id=");
            scanf("%d", &id);

            // validating ID
            if (id < 0)
                puts("Please enter a non-negative number for ID\n");
            else if (id == 0)
                puts("Please enter a number for ID\n");
            else
            {
                // constructing the url for the specific book
                char *url = construct_book_url("/api/v1/tema/library/books/", id);

                message = compute_delete_request("localhost", url, NULL, NULL, 0, JWT_token);

                // opening connection
                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // showing the status of the request to the user
                if (strstr(response, "Error when decoding tokenn!"))
                    puts("You do not have access to the library!\n");
                else if (strstr(response, "No book was deleted!"))
                    puts("No book was found with the given id\n");
                else if (strstr(response, "200 OK"))
                    puts("Book deleted successfully\n");
                close(sockfd);
                free(url);
                free(message);
                free(response);
            }
        }

        else if (strcmp(cmd, "logout") == 0)
        {
            char *message;
            char *response;
            
            // adding current cookie to the cookies list to be sent to the server.
            // since we only intend to send one cookie, we are setting the list
            // to the cookie value
            char **cookies = &cookie;

            message = compute_get_request("localhost", "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL);

            // opening connection
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // showing the status of the request to the user and setting the
            // cookie and JWT token to EMPTY
            if (strstr(response, "error"))
                puts("You are not logged in!\n");
            else if (strstr(response, "200 OK"))
            {
                free(cookie);
                cookie = EMPTY;

                if (strcmp(JWT_token, EMPTY) != 0)
                {
                    free(JWT_token);
                    JWT_token = EMPTY;
                }

                puts("Logged out successfully\n");
            }
            close(sockfd);
            free(message);
            free(response);
        }
    }

    return 0;
}
