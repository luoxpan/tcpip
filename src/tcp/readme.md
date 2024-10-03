hello:

`g++ hello_server.cpp -o hello_server`

`g++ hello_client.cpp -o hello_client`

`./hello_server 2222`

另起一个终端

`./hello_client 127.0.0.1 2222`

echo:

`g++ echo_server.cpp -o echo_server`

`g++ echo_client.cpp -o echo_client`

`./echo_server 2223`

另起一个终端

`./echo_client 127.0.0.1 2223`