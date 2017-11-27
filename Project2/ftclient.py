#!/usr/bin/env python3

"""
This file stores code for the file transfer client for my Project Two assignment for CS 372 at OSU

Name: Alexander Miranda
Assignment: Project 2 File Transfer Client/Server
Due Date: 11/26/2017
Course: CS 372 Introduction to Networking
"""
from os import path
from struct import *
from time import sleep
import socket
import sys

def server_handshake(host, port):
    """
    Method that establishes a connection to the server process

    host - {String} The host name where the server process resides
    port - {Integer} The port number the server process listens to

    returns A file descriptor denoting the socket
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    return sock

def send_number(s, msg):
    """
    Method that transmits a number through the socket connection

    s - {String} The unix file descriptor for the socket connection
    msg - {Number} The number being sent through the socket connection
    """
    sent_data = pack('i', msg)
    s.send(sent_data)

def send_message(s, msg):
    """
    Method that transmits a text message through the socket connection

    s - {String} The unix file descriptor for the socket connection
    msg - {String} The string message that will be encoded and sent
        through the socket connection
    """
    sent_data = bytes(msg, encoding="UTF-8")
    s.sendall(sent_data)

def get_message(s):
    """
    Retrieves the message sent through the socket connection from the server

    s - {String} The unix file descriptor for the socket connection

    returns - {String} The string that was sent from the server to the client
    """
    size = unpack("I", s.recv(4))

    return receive_all(s, size[0])

def show_dir(s):
    """
    Decodes the file directory information and outputs the directory structure
        to the user on the client side
    
    s - {String} The unix file descriptor for the socket connection
    """
    data_size = unpack("I", s.recv(4))
    retrieved = str(s.recv(data_size[0]), encoding="UTF-8").split("\x00")

    for f in retrieved:
        print(f)

def receive_all(s, byte_count):
    """
    Method that takes all stringified data chunks and concates them into a 
    single string and returns that result

    s - {String} The unix file descriptor for the socket connection
    byte_count - {Integer} The data amount of the data requested in terms
        of bytes

    returns {String} - The concated string from the data retrieved from the server
    """
    retrieved = ""

    while len(retrieved) < byte_count:
        data_packet = str(s.recv(byte_count - len(retrieved)), encoding="UTF-8")

        if not data_packet:
            return None
        
        retrieved += data_packet

    return retrieved

def initiate_request(connect, command, port_num):
    """
    Wrapper method that sends both the number and message requests to the server

    connect - {String} The unix file descriptor for the socket connection
    command - {String} The command text for the server to interpret
    port_num - {Integer} An integer denoting the port number the client runs on
    """
    send_message(connect, command + "\0")
    send_number(connect, port_num)

def get_file(connect, file_name):
    """
    Method that writes a copy of the requested file to the working directory
    of the client and will append _copy.txt to the filename.

    connect - {String} The unix file descriptor for the socket connection
    file_name - {String} The name of the file requested that exists on the server
    """
    if path.isfile(file_name):
        file_name = file_name.split(".")[0] + "_copy.txt"

    with open(file_name, 'w') as f_ptr:
        f_ptr.write(buffer)


if __name__ == "__main__":
    """
    The main process for the ftclient when run as a command-line executable
    """
    args_len = len(sys.argv)

    if args_len < 5 or args_len > 6:
        print("Usage: python3 ftclient.py <hostname> <port_number> <command> <file_name> <data_port>")
        exit(1)

    host = sys.argv[1]
    port_num = int(sys.argv[2])
    cmd = sys.argv[3]
    data_port = 0
    file_name = ""

    if len(sys.argv) == 5:
        data_port = int(sys.argv[4])
    elif len(sys.argv) == 6:
        file_name = sys.argv[4]
        data_port = int(sys.argv[5])

    if cmd not in ["-g", "-l"]:
        raise ValueError("Command must be either -g or -l!")

    server = server_handshake(host, port_num)
    initiate_request(server, cmd, data_port)

    # Handling the directory display case
    if cmd == "-l":
        sleep(2)
        data_sock = server_handshake(host, data_port)
        print("Retrieving directory contents from {}: {}".format(host, data_port))
        show_dir(data_sock)
        data_sock.close()
    # Handling the file download case
    elif cmd == "-g":
        send_number(server, len(file_name))
        send_message(server, file_name + "\0")

        response = get_message(server)

        if response == "FILE NOT FOUND!":
            print("{}: {} says {}".format(host, port_num, response))
        elif response == "FILE FOUND!":
            print("Retrieving \"{}\" from {}: {}".format(file_name, host, data_port))
            sleep(2)
            data_sock = server_handshake(host, data_port)
            get_file(data_sock, file_name)
            print("File transfer successful!")
            data_sock.close()

    server.close()