#!/usr/bin/env python3

"""
This file stores code for the file transfer client for my Project Two assignment for CS 372 at OSU

Name: Alexander Miranda
Assignment: Project 2 File Transfer Client/Server
Due Date: 11/26/2017


"""
from os import path
from struct import *
from time import sleep
import socket
import sys

def server_handshake(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    return sock

def send_number(s, msg):
    sent_data = pack('i', msg)
    s.send(sent_data)

def send_message(s, msg):
    sent_data = bytes(msg, encoding="UTF-8")
    s.sendall(sent_data)

def get_message(s):
    size = unpack("I", s.recv(4))

    return receive_all(s, size[0])

def show_dir(s):
    data_size = unpack("I", s.recv(4))
    retrieved = str(s.recv(data_size[0]), encoding="UTF-8").split("\x00")

    for f in retrieved:
        print(f)

def receive_all(s, byte_count):
    retrieved = ""

    while len(retrieved) < byte_count:
        data_packet = str(s.recv(byte_count - len(retrieved)), encoding="UTF-8")

        if not data_packet:
            return None
        
        retrieved += data_packet

    return retrieved

def initiate_request(connect, command, port_num):
    send_message(connect, command + "\0")
    send_number(connect, port_num)

def get_file(connect, file_name):
    if path.isfile(file_name):
        file_name = file_name.split(".")[0] + "_copy.txt"

    with open(file_name, 'w') as f_ptr:
        f_ptr.write(buffer)


if __name__ == "__main__":
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

    if cmd == "-l":
        sleep(2)
        data_sock = server_handshake(host, data_port)
        print("Retrieving directory contents from {}: {}".format(host, data_port))
        show_dir(data_sock)
        data_sock.close()
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