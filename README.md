### Multi-threaded chatting app written in C. 
It uses the TCP sockets to establish connections between multiple clients and the server. 

The server acts as a bridge between clients

The server can handle multiple client requests concurrently, with each client request running on a separate thread.

The server maintains a list of all connected clients, storing relevant information such as the client’s socket ID, user ID, name, surname, and telephone number.

This information is stored in a struct called ‘client‘, and an array of these structs is maintained to keep track of all connected clients.
