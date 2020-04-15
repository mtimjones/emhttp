# emhttp
Embeddable HTTP Server

[This source was from an article I wrote for Dr. Dobb's in 2001.]

This package comprises the Embeddable HTTP Server.  The package is made up
of three parts:

  ./emhttp contains the embeddable HTTP server source,
  ./emhttp/tools contains the internal filesystem creation tool, and
  ./emhttp/tools/root contains a sample content subdirectory.

Quick Build Summary:

This server is provided as a test tool which runs on a standard Linux desktop.
This is useful for building a test version of the server before applying to
the embedded device.

Once the content is created for the device, the 'buildfs' tool is run on the
full pathname of the subdirectory to create a compilable data structure that
represents the file system.  The resulting file, 'filedata.c' is then copied
to the ./emhttp subdirectory and then the server built by simply typing 'make'.

To review content contained within the server, the following may be done
(assuming the server is on a host with the IP address 192.168.1.1):

	http://192.168.1.1:8080/index.html

The usage log may be reviewed by:

	http://192.168.1.1:8080/log

If the browser is on the same host as the emhttp server:

	http://localhost:8080/

Depending upon the socket library provided by the particular RTOS, some
calls may need to be modified.

M. Tim Jones
mtj@mtjones.com

https://drdobbs.com/web-development/an-embeddable-http-server/184404808
