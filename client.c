#include "helpers.h"
#include "request.h"
#include "parson.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h> 
#include <stdlib.h>    
#include <string.h>  
#include <sys/socket.h>
#include <unistd.h> 

char * cookie_find(char * response) {
  int i;
  char cookie[100];
  char * p = strstr(response, "connect.sid=");
  char * t = strchr(p, ';');
  *(p + (t - p)) = '\0';
  return p;
}

int main() {

  char * request, * response;
  int sockfd;
  int weatherfd;

  char * data[1];

  sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
	return -1;
  }

  char * cookies[50];
  int nr = 0;
  char command[20], token[300], info[300];
  char username[20], password[20];
  int has_token = 0;

  while (1) {
	sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
	  return -1;
	}
	scanf("%s", command);
	if (strcmp(command, "register") == 0) {

	  fprintf(stdout, "username=");
	  scanf("%s", username);

	  fprintf(stdout, "password=");
	  scanf("%s", password);

	  strcpy(info, "{\"username\":\"");
	  strcat(info, username);
	  strcat(info, "\",\"password\":\"");
	  strcat(info, password);
	  strcat(info, "\"}");

	  JSON_Value * json = json_parse_string(info);

	  request = compute_post_request("34.254.242.81", "/api/v1/tema/auth/register",
		"application/json", json,
		NULL, 0, NULL);
	  send_to_server(sockfd, request);
	  free(request);
	  response = receive_from_server(sockfd);
	  if (strstr(response, "is taken"))
		printf("%s\n", "This username is not avaible");
	  else
		printf("%s\n", "The user was successfully added");
	}

	if (strcmp(command, "login") == 0) {
	  char cookie[100];
	  fprintf(stdout, "username=");
	  scanf("%s", username);
	  fprintf(stdout, "password=");
	  scanf("%s", password);

	  strcpy(info, "{\"username\":\"");
	  strcat(info, username);
	  strcat(info, "\",\"password\":\"");
	  strcat(info, password);
	  strcat(info, "\"}");
	  JSON_Value * json = json_parse_string(info);

	  request = compute_post_request("34.254.242.81", "/api/v1/tema/auth/login",
		"application/json", json,
		NULL, 0, NULL);

	  send_to_server(sockfd, request);
	  free(request);
	  response = receive_from_server(sockfd);

	  if (strstr(response, "No account with this username"))
		printf("%s\n", "No account with this username");
	  else {
		printf("%s\n", "Welcome! You are logged in");
		strcpy(cookie, cookie_find(response));
		cookies[nr] = malloc(strlen(cookie) + 1);
		strcpy(cookies[nr], cookie);
		nr++;
	  }
	}

	if (strcmp(command, "enter_library") == 0) {
	  if (nr > 0) {
		printf("%s\n", "You have access to the library");
		request = compute_get_request("34.254.242.81", "/api/v1/tema/library/access", NULL,
		  cookies, nr, NULL);
		send_to_server(sockfd, request);
		free(request);

		response = receive_from_server(sockfd);

		JSON_Value * json = json_parse_string(basic_extract_json_response(response));
		strcpy(token, json_object_get_string(json_object(json), "token"));
		has_token = 1;
	  } else
		printf("%s\n", "Library can't be accessed");
	  free(response);
	}

	if (strcmp(command, "add_book") == 0) {
	  if (has_token) {
		char title[20], author[20], page_count[20], genre[20], publisher[20];

		fprintf(stdout, "title=");
		scanf("%s", title);

		fprintf(stdout, "author=");
		scanf("%s", author);

		fprintf(stdout, "genre=");
		scanf("%s", genre);

		fprintf(stdout, "publisher=");
		scanf("%s", publisher);

		fprintf(stdout, "page_count=");
		scanf("%s", page_count);

		if (strlen(title) <= 1 || strlen(author) <= 1 || strlen(publisher) <= 1 ||
		  strlen(page_count) <= 0 || strlen(genre) <= 1)
		  printf("%s\n", "Invalid book");
		else {
		  int valid = 1;
		  for (int i = 0; i < strlen(page_count); i++)
			if (page_count[i] < '0' || page_count[i] > '9') {
			  printf("%s\n", "Invalid book");
			  valid = 0;
			  break;
			}

		  if (valid == 1) {
			printf("%s\n", "The book was added");
			strcpy(info, "{\"title\": \"");
			strcat(info, title);
			strcat(info, "\",\"author\": \"");
			strcat(info, author);
			strcat(info, "\",\"genre\": \"");
			strcat(info, genre);
			strcat(info, "\",\"page_count\": ");
			strcat(info, page_count);
			strcat(info, ",\"publisher\": \"");
			strcat(info, publisher);
			strcat(info, "\"}");

			JSON_Value * json = json_parse_string(info);
			request = compute_post_request("34.254.242.81", "/api/v1/tema/library/books",
			  "application/json", json,
			  NULL, 0, token);

			send_to_server(sockfd, request);
			free(request);
			response = receive_from_server(sockfd);
		  }
		}
	  } else
		printf("%s\n", "You don't have access to add this book");
	}

	if (strcmp(command, "get_books") == 0) {
	  if (has_token) {
		request = compute_get_request("34.254.242.81", "/api/v1/tema/library/books", NULL,
		  cookies, nr, token);
		send_to_server(sockfd, request);
		free(request);
		response = receive_from_server(sockfd);
		if (strchr(response, '{'))
		  printf("[%s\n", basic_extract_json_response(response));
		else
		  printf("%s\n", "The list of books is empty");
	  } else
		printf("%s\n", "Books can't be accessed");
	}

	if (strcmp(command, "get_book") == 0) {
	  char id[10], path[30];
	  if (has_token) {
		printf("id=");
		scanf("%s", id);
		strcpy(path, "/api/v1/tema/library/books");
		strcat(path, "/");
		strcat(path, id);
		request = compute_get_request("34.254.242.81", path, NULL,
		  cookies, nr, token);

		send_to_server(sockfd, request);
		free(request);
		response = receive_from_server(sockfd);
		if (strstr(response, "No book was found"))
		  printf("%s\n", "No book with that ID");
		else
		  printf("%s\n", basic_extract_json_response(response));
	  } else
		printf("%s\n", "Book can't be accessed");
	}

	if (strcmp(command, "delete_book") == 0) {
	  if (has_token) {
		char id[10], path[30];
		printf("id=");
		scanf("%s", id);
		strcpy(path, "/api/v1/tema/library/books");
		strcat(path, "/");
		strcat(path, id);
		request = compute_delete_request("34.254.242.81", path, NULL,
		  cookies, nr, token);
		send_to_server(sockfd, request);
		free(request);
		response = receive_from_server(sockfd);
		if (strstr(response, "error"))
		  printf("%s\n", "No book with that ID");
		else
		  printf("%s\n", "The book was deleted");
	  } else
		printf("%s\n", "Book can't be accessed");

	}

	if (strcmp(command, "logout") == 0) {
	  if (nr > 0) {
		request = compute_get_request("34.254.242.81", "/api/v1/tema/auth/logout", NULL,
		  cookies, nr, token);
		send_to_server(sockfd, request);
		free(request);
		response = receive_from_server(sockfd);
		has_token = 0;
		nr = 0;
		free(cookies[0]);
		printf("%s\n", "User disconnected");
	  } else
		printf("%s\n", "You are not logged in");
	}

	if (strcmp(command, "exit") == 0) {
	  break;
	}

	close(sockfd);
  }
  return 0;
}