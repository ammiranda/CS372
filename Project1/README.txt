Instructions for chat_server:

Starting the server:

    python chat_server <PORT_NUMBER>
    Enter a user name of 10 characters or less: <SERVER_USER_NAME>

After the server started it will be waiting for a client connection

Starting the client:

    make
    ./chat_server <HOST_NAME/IP> <PORT_NUMBER>
    Please enter a username of 10 characters or less: <CLIENT_USER_NAME>
    <CLIENT_USER_NAME>> <Initial chat message to server>

Once the client is running the chat server and chat client can send alternating messages to each other.

Either the server or client can type '\quit' to close the connected client connection.

The server will then wait for another client to connect until a SIGINT signal is intercepted.
