#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <cstdio>
using namespace std;

const string SERVER_STRING = "Server: jdbhttpd/0.1.0\r\n";
class Server {
	private:
		sockaddr_in sin;
		int server_sock;
		int port;
		bool portFlag;
	public:
		Server(int myPort) : port(myPort),server_sock(-1),portFlag(true) {};
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
				portFlag = false;
				return;
			}	
			if(listen(server_sock, 5) < 0)
				cout << "listen error" << endl;
		}

		int get_line(int sock, string &buf) {
			buf.clear();
			int len = 0;
			char tmp = '\0';
			int n;
			while( tmp != '\n') {
				n = recv(sock, &tmp, 1, 0); 
				if(n > 0) {
					if(tmp == '\r') {
						n = recv(sock, &tmp, 1, MSG_PEEK);
						if( n > 0 && tmp == '\n')
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
			int cgi = 0;
			struct stat st;
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
			string queryStr = url;
			if(method == "GET") {
				int queryBeginIndex = 0;
				while(queryStr[queryBeginIndex] != '?' && queryBeginIndex < queryStr.length()) queryBeginIndex++;
				if(queryBeginIndex < queryStr.length() && queryStr[queryBeginIndex] == '?') {
					queryStr = queryStr.substr(queryBeginIndex+1);
					cgi = 1;
				}
			}

			string resourcePath = "web_resource" + url;
			if(resourcePath[resourcePath.size() - 1] == '/')
				resourcePath += "index.html"; //default path 

					if(stat(resourcePath.c_str(), &st) == -1) {
						while(numchar != 0 && buf != "\n")
							numchar = get_line(client_sock, buf);
						cout << "not found !" << endl;			
					} else {
						if((st.st_mode & S_IFMT) == S_IFDIR)
							resourcePath += "index.html";

						if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) 
							cgi = 1;

						if(cgi == 0)
							serve_file(client_sock, resourcePath);
						else	
							;
					}
			close(client_sock);
		}

		void serve_file(int sock, string path) {
			cout << "serve file ! " + path << endl;
			FILE *resource = NULL;
			int numchar = 1;
			string buf = "A";
			while(numchar > 0 && buf != "\n" ) {
				numchar = get_line(sock, buf); //read & discard headers
			}
			resource = fopen(path.c_str(), "r");
			if(resource == NULL) {
				cout << "not found" << endl;
			} else {
				headers(sock, path);
				cat(sock, resource);
			}
			fclose(resource);
		}
		
		void headers(int sock, string filename) {
			cout << "HEADERS" << endl;
			string buf;
			(void)filename;  //unknown functioe

			buf = "HTTP/1.0 200 OK\r\n";
			send(sock, buf.c_str(), buf.length(), 0);
			buf = SERVER_STRING;
			send(sock, buf.c_str(), buf.length(), 0);
			buf = "Content-Type: text/html\r\n";
			send(sock, buf.c_str(), buf.length(), 0);
			buf = "\r\n";
			send(sock, buf.c_str(), buf.length(), 0);
		}
		
		void cat(int sock, FILE *resource) {
			char buf[1024];
			memset(buf, 0, sizeof(buf));
			fgets(buf, sizeof(buf), resource);
			while(!feof(resource)) {
				send(sock, buf, sizeof(buf), 0);
				memset(buf, 0, sizeof(buf));
				fgets(buf, sizeof(buf), resource);
			}
		}

		void run() {
			if(server_sock == -1 || portFlag == false) {
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
