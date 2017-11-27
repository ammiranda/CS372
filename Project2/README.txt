CS 372 Introduction to Networking Project Two

Running the server code do the following commands in the root of this directory:

* make
* ./ftserver <port number>

The server process will then be outputting to the current terminal window. You will need to open
another terminal window in order to run the ftclient.py as shown below:

Each secondary bullet point are their own arguments to the following command:
* python3 ftclient.py 
    * <hostname (will be 127.0.0.1 if server is same host)> 
    * <port number>
    * <command (-l or -g)>
    * <file name>
    * <data port number>

The ftclient will stop after either the directory data is displayed or the file transfer completes or errors.
The ftserver will run continually until a SIGINT is sent to the server process.

The ftclient and ftserver code were tested on os1.engr.oregonstate.edu and flip1.engr.oregonstate.edu.

In order to clean the existing server executable and delete text file copies run:

* make clean