#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>

int clientCount = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct client
{

	int index;
	int sockID;
	int userID;
	char name[20];
	char surname[20];
	char telephone[20];
	struct sockaddr_in clientAddr;
	int len;
};

struct client Client[1024];
pthread_t thread[1024];

int getUserByIdIfExist(int userID);
void *doNetworking(void *ClientDetail);
void writeToHistoryFile(int client_userID, int dest_userID, char *message);
void handleConfig(int index, int clientSocket, char *data);
void handleAdd(int index, int clientSocket, char *data);
void handleSend(int index, int clientSocket, char* data);
void handleHistory(int index, int clientSocket, char* data);

int main()
{

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0); // PF_INET = IPv4 , SOCK_STREAM = TCP , 0 = IP

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		return 0;

	if (listen(serverSocket, 1024) == -1)
		return 0;

	printf("Server started listenting on port 8080 ...........\n");

	while (1)
	{

		Client[clientCount].sockID = accept(serverSocket, (struct sockaddr *)&Client[clientCount].clientAddr, &Client[clientCount].len);
		Client[clientCount].index = clientCount;

		pthread_create(&thread[clientCount], NULL, doNetworking, (void *)&Client[clientCount]);

		clientCount++;
	}

	for (int i = 0; i < clientCount; i++)
		pthread_join(thread[i], NULL);
}

void *doNetworking(void *ClientDetail)
{

	struct client *clientDetail = (struct client *)ClientDetail;
	int index = clientDetail->index;
	int clientSocket = clientDetail->sockID;

	printf("Client %d connected.\n", index + 1);

	char output[1024];
	int dest_index;

	while (1)
	{

		char data[1024];
		int read = recv(clientSocket, data, 1024, 0);
		data[read] = '\0';

		if (strcmp(data, "CONFIG") == 0)

		{
			handleConfig(index, clientSocket, data);
		}
		else if (strcmp(data, "ADD") == 0)
		{
			handleAdd(index, clientSocket, data);
		}
		else if (strcmp(data, "SEND") == 0)
		{
			handleSend(index, clientSocket, data);
			
		}
		else if (strcmp(data, "HISTORY") == 0)
		{
			handleHistory(index, clientSocket, data);
		}
		else if (strcmp(data, "LIST") == 0)
		{
			handleList(index, clientSocket);
		}
		else
		{
			printf("Invalid command or user not found\n");
		}
	}

	return NULL;
}

int getUserByIdIfExist(int userID)
{
	for (int i = 0; i < clientCount; i++)
	{

		if (Client[i].userID == userID)
		{
			return i;
		}
	}

	return -1;
}

void writeToHistoryFile(int client_userID, int dest_userID, char *message)
{
	char file_name[12];
	sprintf(file_name, "%d", client_userID);
	strcat(file_name, "/history/");
	char dest_userID_str[12];
	sprintf(dest_userID_str, "%d", dest_userID);
	strcat(file_name, dest_userID_str);
	strcat(file_name, ".txt");
	FILE *fp;
	fp = fopen(file_name, "a+");
	fprintf(fp, "%s\n", message);
	fclose(fp);
}

void handleConfig(int index, int clientSocket, char *data)
{
	printf("Client %d requested configuration.\n", index + 1);

	int read = recv(clientSocket, data, 1024, 0);
	data[read] = '\0';

	int userID = atoi(data);

	Client[index].userID = userID;

	read = recv(clientSocket, data, 1024, 0);
	data[read] = '\0';
	strcpy(Client[index].name, data);

	read = recv(clientSocket, data, 1024, 0);
	data[read] = '\0';
	strcpy(Client[index].surname, data);

	read = recv(clientSocket, data, 1024, 0);
	data[read] = '\0';
	strcpy(Client[index].telephone, data);

	printf("Client %d configured with userID %d, %s, %s %s\n", index + 1, userID, Client[index].name, Client[index].surname, Client[index].telephone);

	// create a folder for client with userID
	char folder_name[12];
	sprintf(folder_name, "%d", Client[index].userID);
	// check if folder exists
	struct stat st = {0};
	if (stat(folder_name, &st) == -1)
	{
		mkdir(folder_name, 0777);
	}
	else
	{
		printf("Folder already exists\n");
	}

	mkdir(strcat(folder_name, "/history"), 0777);
}

void handleAdd(int index, int clientSocket, char *data)
{
	printf("Client %d requested to add a contact.\n", index + 1);

	int read = recv(clientSocket, data, 1024, 0);
	data[read] = '\0';
	int contact_userID = atoi(data);

	// create a file for client's contacts
	char file_name[12];
	sprintf(file_name, "%d", Client[index].userID);
	strcat(file_name, "/contacts.txt");
	FILE *fp;
	fp = fopen(file_name, "a+");

	// check if contact already exists
	int contact_exist_flag = 0;
	char line[1024];
	while (fgets(line, sizeof(line), fp))
	{
		if (strstr(line, data) != NULL)
		{
			contact_exist_flag = 1;
			break;
		}
	}
	if (contact_exist_flag == 0)
	{
		// find the contact's name, surname and telephone
		int contact_index = getUserByIdIfExist(contact_userID);
		fprintf(fp, "%d %s %s %s\n", contact_userID, Client[contact_index].name, Client[contact_index].surname, Client[contact_index].telephone);
	}
	fclose(fp);

	printf("contact_userID assigned as: %d\n", contact_userID);
}

void handleSend(int index, int clientSocket, char* data)
{
    printf("Client %d requested to send a message.\n", index + 1);

    int read = recv(clientSocket, data, 1024, 0);
    data[read] = '\0';

    int dest_userID = atoi(data);
    printf("dest_userID assigned as: %d\n", dest_userID);

    // check if user exists
    int dest_index = getUserByIdIfExist(dest_userID);

    if (dest_index != -1)
    {
        read = recv(clientSocket, data, 1024, 0);
        data[read] = '\0';

        // add sender id to message
        char senderID[1024];
        sprintf(senderID, "%d", Client[index].userID);
        strcat(senderID, ": ");
        strcat(senderID, data);

        writeToHistoryFile(Client[index].userID, dest_userID, senderID);
        writeToHistoryFile(dest_userID, Client[index].userID, senderID);
        send(Client[dest_index].sockID, senderID, 1024, 0);
    }
}

void handleHistory(int index, int clientSocket, char* data)
{
    printf("Client %d requested to view history.\n", index + 1);

    int read = recv(clientSocket, data, 1024, 0);
    data[read] = '\0';

    int dest_userID = atoi(data);
    printf("dest_userID assigned as: %d\n", dest_userID);

    // check if user exists
    int dest_index = getUserByIdIfExist(dest_userID);

    if (dest_index != -1)
    {
        // read history file
        char file_name[12];
        sprintf(file_name, "%d", Client[index].userID);
        strcat(file_name, "/history/");
        char dest_userID_str[12];
        sprintf(dest_userID_str, "%d", dest_userID);
        strcat(file_name, dest_userID_str);
        strcat(file_name, ".txt");
        FILE *fp;

        fp = fopen(file_name, "r");
        if (fp == NULL)
        {
            printf("Failed to open file: %s\n", file_name);
            return;
        }
        char line[1024];
        while (fgets(line, sizeof(line), fp))
        {
            printf("Read line: %s\n", line);
            send(clientSocket, line, 1024, 0);
        }
        fclose(fp);
    }
    else
    {
        send(clientSocket, "User does not exist", 1024, 0);
    }
}

void handleList(int index, int clientSocket)
{
    printf("Client %d requested to list all clients.\n", index + 1);

    char output[1024] = "";
    int l = 0;

    for (int i = 0; i < clientCount; i++)
    {
        if (i != index)
            l += snprintf(output + l, sizeof(output) - l, "Client userID %d -> %s is at socket %d.\n", Client[i].userID, Client[i].name, Client[i].sockID);
    }

    send(clientSocket, output, strlen(output) + 1, 0);
}