#!/bin/python

"""
This file stores code for the chat server for my Project One assignment for CS 372 at OSU

Name: Alexander Miranda
Assignment: Project 1 Chat Client/Server
Due Date: 10/29/2017

References: http://codingnights.com/coding-fully-tested-python-chat-server-using-sockets-part-1/
"""

# Importing necessary dependency libraries
import socket
import sys

# Constant that sets what the max message length based on assignment specification
MAX_MESSAGE_LENGTH = 500

class ChatServer():
    """
    Class for the ChatServer that allows for a chat client to 
    connect and communicate with an instance of this class
    """

    def __init__(self, port=7851):
        """
        Initialization of the ChatServer class and fills out 
        relevant chat server fields has a default port set but will
        be provided by user input
        """
        self.host = ""
        self.port = port
        self.username = ""

    def start(self):
        """
        Method that starts the running of the chat server and maintains the
        connection as well as grabbing the server-side username
        """
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # bind the socket to the port specified by the user
        server_socket.bind((self.host, self.port))
        # listen on the port for incoming messages
        server_socket.listen(1)

        self._grab_username()

        print "Server started and is waiting for incoming client connection(s)"
        while 1:
            connect_sock, addr = server_socket.accept()
            # Output that a connection request occured
            print "received connection on address {}".format(addr)
            self._chat(connect_sock, self._handshake(connect_sock, self.username), self.username)
            # Close the connection when chatting is done
            connect_sock.close()

    def _chat(self, connect_sock, clientname, username):
        """
        Opens a connection to a chat client that allows them to send
        the first message

        connect_sock: The socket on which the connection resides
        clientname: The chat client's username
        username: The chat server's username
        """
        message_payload = ""
        while 1:
            received = connect_sock.recv(MAX_MESSAGE_LENGTH)[0:-1]
            if received == "":
                print "Connection closed for user {0}".format(clientname)
                print "Waiting for new incoming client connection(s)"
                break
            # Output the clientname to the serverside user
            print "{0}> {1}".format(clientname, received)
            # Retrieving the server side message to send to the client-side user
            message_payload = ""
            while len(message_payload) == 0 or len(message_payload) > MAX_MESSAGE_LENGTH:
                message_payload = raw_input("{}> ".format(username))
                # Transmit the server-side message to the chat client
            if message_payload == "\quit":
                print "Connection closed"
                print "Waiting for new connection"
                break
            connect_sock.send(message_payload)

    def _handshake(self, connect_sock, username):
        """
        This method exchanges usernames between client and server while
        establishing the initial chat connection

        connect_sock: The socket on which the connection resides
        username: The chat server's username

        returns: The chat client's username
        """
        # Grab the chat client's name
        clientname = connect_sock.recv(1024)
        # Transmit the server-side username to the chat client
        connect_sock.send(username)

        return clientname

    def _grab_username(self):
        """
        This method grabs the chat server's username from the user
        running the chat server
        """
        while len(self.username) > 10 or len(self.username) == 0:
            self.username = raw_input("Enter a user name of 10 characters or less: ")

def main(port_num):
    """
    Main routine that initializes a ChatServer object and runs its start method
    """
    chat_server = ChatServer(port_num)
    chat_server.start()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Port number for the server must be specified in order to run properly"
        exit(1)

    main(int(sys.argv[1]))