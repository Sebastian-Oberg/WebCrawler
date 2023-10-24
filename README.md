# WebCrawler

This is a project focused on creating a simple web client using Visual C++. The project aims to enhance my understanding of text-based application-layer protocols, multithreading, system APIs, and Windows sockets.

## Purpose

The purpose of this project is to develop a basic web client capable of accepting URLs and extracting server/page statistics. This involves establishing connections to web servers, sending requests, and processing responses.

## Technical Details

### Command-Line Argument Handling

The program is designed to accept a single command-line argument, which should be a target URL. If the argument is either missing or there are too many arguments provided, the program will display usage information and terminate.

### URL Format

The program expects URLs in the following format:
```
scheme://host[:port][/path][?query][#fragment]
```
For this project, only the "http" scheme is considered. Examples of valid URLs include:
- `http://www.example.com`
- `http://www.example.com:80`
- `http://192.168.1.1:8080/page/index.html`

### Network Communication

The program uses Windows sockets for network communication. It creates a socket and establishes a connection to the specified host on the provided port. If the connection is successful, it sends an HTTP 1.0 request to the server.

### Receive Loop

Data is read from the socket using a receive loop. The program waits to see if the socket has any data using the `select` function. If new data is available, it reads the next segment using `recv`. The buffer is dynamically resized as needed to accommodate larger responses.

### HTTP Request Format

The program sends a GET request to the server. At a minimum, the request includes the request line and the host string with the name of the server. Additionally, it may request that the server close the connection using the "Connection: close" header. It also includes a user-agent string to identify the client.

### Parsing Responses

If a valid reply with a status code in the 2xx range is received, the program proceeds to parse the HTML result and display the required information about the download. This includes details such as timing information, HTTP header, and number of links.

### Handling Errors

If there are errors during any phase (connection, receiving data, parsing), the program gracefully reports the type of error encountered and terminates without crashing. It also handles cases where the remote host is unresponsive.

## Additional Considerations

- The program efficiently handles buffer resizing to accommodate pages of arbitrary length.
- It uses appropriate timing functions (e.g., `clock()`) to measure the duration of different networking steps.
- The program ensures that potential issues like buffer overflows, access violations, and memory leaks are avoided.
- It is robust against unexpected responses from the Internet and avoids deadlocks in socket communication.

## Future Enhancements

For future iterations of this project, there are several areas that can be enhanced to further improve its functionality and performance. These enhancements can contribute to a more robust and efficient web client.

### Multi-Threaded URL Downloading

In this phase, the program will be extended to download multiple URLs from an input file using a single thread. This will allow for more efficient use of resources and faster retrieval of information. The program will implement measures to ensure politeness by only accessing unique IPs and checking the existence of robots.txt files.

### Enhanced URL Parsing and Handling

- **Improved URL Parsing:** Enhance the URL parsing mechanism to support a wider range of URL formats, including variations in schemes, ports, and query parameters. This will increase the program's versatility and compatibility with different types of URLs.

- **Dynamic Buffer Handling:** Implement a more sophisticated approach to handling page buffers. Instead of hardwiring a fixed buffer size, dynamically allocate memory based on the actual page size. This will optimize RAM usage and improve scalability for larger-scale operations.

### Enhanced Politeness and Efficiency

- **Parallelization:** Explore opportunities for parallelization to further improve efficiency. Consider using multiple threads to handle simultaneous connections and downloads, especially when dealing with a large number of URLs.

- **Request Optimization:** Implement strategies to optimize HTTP requests, such as utilizing persistent connections or implementing request headers to indicate the desired behavior (e.g., closing the connection after the response).

### Handling Errors and Edge Cases

- **Error Logging and Reporting:** Enhance the program's ability to log and report errors. Provide detailed information about the nature of errors, including DNS lookup failures, connection timeouts, and non-HTTP responses.

- **Graceful Error Handling:** Ensure that the program gracefully handles errors and failures without crashing. Implement robust error-checking mechanisms at every stage of the process.

### Memory Management and Resource Cleanup

- **Efficient Buffer Reuse:** Optimize the management of page buffers to minimize memory consumption. Implement strategies for reusing and reallocating buffers based on the size of the received content.

- **Resource Cleanup:** Ensure proper cleanup of resources, such as closing sockets and releasing memory, to prevent memory leaks and maintain system stability.

### Scalability and Performance Optimization

- **Performance Profiling:** Conduct performance profiling and analysis to identify bottlenecks and areas for optimization. Use profiling tools to measure execution times and resource utilization.

- **Optimized Algorithm Design:** Evaluate and refine algorithms to achieve better time and space complexity, especially for tasks like DNS lookups, connection establishment, and data retrieval.
