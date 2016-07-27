#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
using namespace std;

class Server {
	private:
		sockaddr_in sin;
		int server_sock;
		int port;
	public:
	Server(int myPort) : port(myPort),server_sock(-1) {};
	void start() {
		server_sock = socket(PF_INET, SOCK_STREAM, 0);
		if(server_sock == -1)
			cout << "fail to establish a sever" << endl;
		memset(&sin, 0x00, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = htonl(INADDR_ANY);	
		if(bind(server_sock, (sockaddr*)&sin, sizeof(sin)) < 0) {
			cout << "bind error " << endl;	
		}	
		if(listen(server_sock, 5) < 0)
			cout << "listen error" << endl;
	}
	
int get_line(int sock, string &buf) {
		int len = 0;
		char tmp = '\0';
		int n;
		while( tmp != '\n') {
			n = recv(sock, &tmp, 1, 0); 
			if(n > 0) {
				if(tmp == '\r') {
					n = recv(sock, &tmp, 1, MSG_PEEK);
					if( n > 0 && n == '\n')
						n = recv(sock, &tmp, 1, 0);
					else
						tmp = '\n';
				}
				buf.push_back(tmp);
			}
		}
		return buf.size();
	}

	void accept_request(int client_sock) {
		string buf, method, url;
		int numchar = get_line(client_sock, buf);
		int i = 0;
		while(buf[i] != ' ') {
			method.push_back(buf[i++]);
		}	
	
		cout << "method type: " << method << endl;
		if(method != "POST" && method != "GET") {
			cout << "unknown request" << endl;
			return;
		}

		while(buf[i] == ' ')
			i++;

		while(buf[i] != ' ')
			url.push_back(buf[i++]);

		cout << "request url: " << url << endl;

	}

	void run() {
		if(server_sock == -1) {
			cout << "server not open" << endl;
			return;
		}

		sockaddr_in clt;
		int client_sock ;
		unsigned int client_size = sizeof(clt);

		while(1) {
			client_sock = accept(server_sock, (struct sockaddr*)&clt, &client_size);
			if(client_sock == -1) {
				cout << "accept fail" << endl;
			}
			accept_request(client_sock);
		}
		close(server_sock);
	}
};

int main() {
	Server myServer(8080);
	myServer.start();
	myServer.run();	
	return 0;
}
