#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define exit(N) {fflush(stdout); fflush(stderr); _exit(N); }

#define RMAX 4096

static ssize_t RSIZE = 0;//how many bytes are in the request message
static char request[RMAX + 1];//may need to +1 here for null byte


static int get_port(void);

struct Server_stats {
    int requests;
    int header_bytes;
    int body_bytes;
    int errors;
    int error_bytes;
};

struct Req1 {
    int rsize;
    int fd;
    char requestBuffer[RMAX];
    int headerSent;//0 means header has not been sent, 1 means header sent already
    int allSent; //0 means not all data has been sent, 1 means all data has been sent
    int needsMult;//1 means needs to send body in multiple sends, 0 means not
    int fileopen;// fd that was opened
    int numberOfBytes;//number of bytes of request
    int index;//index in linked list
    struct Req1* next;//next pointer
    

};

struct Req1* headOfArray;
int req1ArrayCount = 0;
//Wrapper functions for Socket, Bind, Listen, Accept

static void addRequest(int rsize1,int clientfd,char* requestBuffer,int headerSent,int allSent,int needsMult, int fileopen, int numberOfBytes) {
    struct Req1* newNode = (struct Req1*)malloc(sizeof(struct Req1));
    if(headOfArray == NULL) {
        //first element to array
        newNode->rsize = rsize1;
        newNode->fd = clientfd;
        memcpy(newNode->requestBuffer,requestBuffer,rsize1);
        newNode->requestBuffer[rsize1] = '\0';
        newNode->headerSent = headerSent;
        newNode->allSent = allSent;
        newNode->needsMult = needsMult;
        newNode->fileopen = fileopen;
        newNode->numberOfBytes = numberOfBytes;
        newNode->index = 0;
        newNode->next = NULL;
        headOfArray = newNode;
        
    } else {
        int index = 1;
        newNode->rsize = rsize1;
        newNode->fd = clientfd;
        memcpy(newNode->requestBuffer,requestBuffer,rsize1);
        newNode->requestBuffer[rsize1] = '\0';
        newNode->headerSent = headerSent;
        newNode->allSent = allSent;
        newNode->needsMult = needsMult;
        newNode->fileopen = fileopen;
        newNode->numberOfBytes = numberOfBytes;
        newNode->next = NULL;
        struct Req1* pointer = headOfArray;
        while(pointer->next != NULL) {
            pointer = pointer->next;
            index++;
        }
        newNode->index = index;
        pointer->next = newNode;
    }
}
static void removeRequest(int index) {
    struct Req1* pointer = headOfArray;
    if(pointer->index == index) {
        //need to remove head
        struct Req1* nodeToRemove = headOfArray;
        headOfArray = headOfArray->next;
        nodeToRemove->next = NULL;
        free(nodeToRemove);
        return;
    }

    struct Req1* previous = headOfArray;
    while(pointer->next != NULL) {
        pointer = pointer->next;
        if(pointer->index == index) {
            struct Req1* nodeToRemove = pointer;
            previous->next = nodeToRemove->next;
            free(nodeToRemove);
            break;
        }
        previous = previous->next;
    }
    return;
}


static int Socket(int namespace, int style, int protocol) {
    
    int sockfd = socket(namespace,style,protocol);
    if(sockfd < 0) {
        perror("socket");
        exit(1);
    }
    return sockfd;
}

static void Bind(int sockfd, struct sockaddr *server, socklen_t length) {
    if(bind(sockfd,server,length) < 0) {
        perror("bind");
        exit(1);
    }
}
static void Listen(int sockfd, int qlen) {

    if(listen(sockfd,qlen) < 0) {
        perror("listen");
        exit(1);
    }
}
static int Accept(int sockfd, struct sockaddr* addr, socklen_t * length_ptr) {
    int newfd = accept(sockfd,addr,length_ptr);
    if(newfd < 0) {
        perror("accept");
        exit(1);
    }
    return newfd;
}
ssize_t Recv(int socket, void* buffer, size_t size, int flags) {
    ssize_t ret_size = recv(socket,buffer,size,flags);
    if(ret_size < 0) {
        perror("recv");
        exit(1);
    }
    return ret_size;
}
ssize_t Send(int socket, void* buffer, size_t size, int flags) {
    ssize_t ret_size = send(socket,buffer,size,flags);
    if(ret_size < 0) {
        perror("send");
        exit(1);
    }
    return ret_size;
}


static void send_chunk(int clientfd, int file_fd, int content_length, struct Server_stats *stats2,int indexOfStruct) {
    struct Req1 *r1 = headOfArray;
    while(r1->next != NULL) {
        if(r1->index == indexOfStruct) {
            break;
        }
        r1 = r1->next;
    }
    int chunksize = 1024;

    int bytesRead = 0;
    char body[chunksize];
    if((bytesRead = read(file_fd, body, chunksize)) > 0){
        Send(clientfd, body, bytesRead, 0); 
        stats2->body_bytes += bytesRead;
    } else {
        r1->allSent = 1;
    }
    
    return;
}

static void handle_request(int clientfd, char* savedArray, int* lenSavedArray, struct Server_stats *stats2,int indexOfStruct, int* fileopen,int*sizeofFile) {
    struct Req1 *r1 = headOfArray;
    while(r1->next != NULL) {//find the struct = request
        if(r1->index == indexOfStruct) {
            break;
        }
        r1 = r1->next;
    }
     
    (*sizeofFile) = 0;
    r1->needsMult = 0;
    int responseType = 0;
    int rsize_ = r1->rsize;
    char req[RMAX];
    memcpy(req,r1->requestBuffer,rsize_);
    if(rsize_ == 0) {
        
        close(clientfd);
        return;
    }
    req[rsize_] = '\0';
    int hasBody = 0;
    char delim[] = " \r\n";
    char delim1[] = " ";
    char delim2[] = "\r\n";
    
    
    //find the body of the request
    char *ptr = strstr(req, "\r\n\r\n");
    
    char response[RMAX] = "";
    char *method = strtok(req, delim1);//Method
    int methodIsNull = 0;
    if(method == NULL) {
        methodIsNull = 1;
        responseType = 1;
        goto responseType;
    }
    char* URI = NULL;
    URI = strtok(NULL, delim1);// uri
    
    int URI_isNull = 0;
    if(URI == NULL) {
        URI_isNull = 1;
        URI = method;
    }
    char* version = strtok(NULL,delim2); //gets the version
    
    int version_isNull = 0;
    if(version == NULL) {
        version = URI;
        if(URI == NULL) {
            version_isNull = 1;
        } else {
            version_isNull = 0;
        }
        
    }
    
    char req_headers[RMAX];
    req_headers[0] = '\0';

    if(ptr == NULL) {
        responseType = 1;
        goto responseType;
    }
    char* next;
    if(version_isNull == 0) {
        next = version + strlen(version) + 2;//points to the headers
        *(ptr) = '\0';
        strcat(req_headers,next);
        ptr = ptr + 4;//is the body
    }
    

    
    //ptr is the request_body
    

    //Creating the response variables
    int contentLength = 0;
    char body[RMAX];
    char headers[RMAX];
    body[0] = '\0';
    headers[0] = '\0';
    
    int needMultipleSends = 0;
    int fd = -1;

    if(strcmp(method, "GET") == 0) {
        if(strcmp(URI, "/ping") == 0) {
            body[0] = '\0';
            strcat(body,"pong");
            contentLength = strlen(body);
            char header[40] = "Content-Length: ";
            sprintf(header + strlen(header), "%d", contentLength);
            strcat(headers,header);
            strcat(headers,"\r\n");
        } else if(strcmp(URI, "/echo") == 0) {
            strcat(body,req_headers);
            //response body can't be more than 1024 bytes
            
            contentLength = strlen(body);
            if(contentLength > 1024) {
                responseType = 3;
            } else {
                char header[40] = "Content-Length: ";
                sprintf(header + strlen(header), "%d", contentLength);
                strcat(headers,header);
                strcat(headers,"\r\n");
            }
            
        } else if(strcmp(URI, "/read") == 0) {

            if((*lenSavedArray) == 0) {
                memcpy(body,"<empty>",7);
                contentLength = 7;
            } else {
                memcpy(body,savedArray,*lenSavedArray);
                
                contentLength = *lenSavedArray;
                body[contentLength] = '\0';
            }
            char header[40] = "Content-Length: ";
            sprintf(header + strlen(header), "%d", contentLength);
            strcat(headers,header);
            strcat(headers,"\r\n");
            
        } else if(strcmp(URI, "/stats") == 0) {
            //test case 8
            char string1[RMAX];
            
            strcat(body,"Requests: ");
            sprintf(string1, "%d", stats2->requests);
            strcat(body,string1);
            strcat(body,"\n");
            strcat(body,"Header bytes: ");
            sprintf(string1, "%d", stats2->header_bytes);
            strcat(body,string1);
            strcat(body,"\n");
            strcat(body,"Body bytes: ");
            sprintf(string1, "%d", stats2->body_bytes);
            strcat(body,string1);
            strcat(body,"\n");
            strcat(body,"Errors: ");
            sprintf(string1, "%d", stats2->errors);
            strcat(body,string1);
            strcat(body,"\n");
            strcat(body,"Error bytes: ");
            sprintf(string1, "%d", stats2->error_bytes);
            strcat(body,string1);

            contentLength = strlen(body);

            char header[40] = "Content-Length: ";
            sprintf(header + strlen(header), "%d", contentLength);
            strcat(headers,header);
            strcat(headers,"\r\n");
            

        } else {
            //try to get the file on disk
            char* filename = URI+1;//removes the first /
            fd = open(filename,O_RDONLY,0);
            int errorNum = 0;
            if(fd == -1) {
                errorNum = 1;
            }
            if(errorNum == 0) {
                struct stat file_info;
                fstat(fd,&file_info);
                int file_size = 0;
                int filetype = 0;
                if(S_ISDIR(file_info.st_mode)) {
                    filetype = 1;
                } else if(S_ISLNK(file_info.st_mode)) {
                    filetype = 2;
                } else if(S_ISREG(file_info.st_mode)) {
                    filetype = 3;
                }
                
                if(filetype == 3) {
                    // a regular file, get contents
                    
                    file_size = file_info.st_size;
                    contentLength = file_size;
                    int bytesRead;
                    if(file_size > RMAX) {
                        needMultipleSends = 1;
                        body[0] = '\0';
                        r1->needsMult = 1;
                    } else {//needsMultipleSends set to 1 either way to distinguish as common get request
                        needMultipleSends = 1;
                        r1->needsMult = 1;
                        body[0] = '\0';
                    }
                    
                    
                    char header[40] = "Content-Length: ";
                    sprintf(header + strlen(header), "%d", contentLength);
                    strcat(headers,header);
                    strcat(headers,"\r\n");
                } else {
                    
                    responseType = 2;
                    //respond with 404 not found
                }

            } else {
                
                responseType = 2;
                //respond with 404 not found
            }
            

        }
    } else if(strcmp(method, "POST") == 0) {
        if(strcmp(URI, "/write") == 0) {
            //need to find the contentLength in the header
            char* lengthHeader = strstr(req_headers,"Content-Length");
            char* contentLine = strtok(lengthHeader,delim2);
            char* length1 = contentLine + 16;
            contentLength = atoi(length1);
            if(contentLength > 1024) {
                responseType = 3;
            } else {
                savedArray[0] = '\0';//clear the array since we writing
                *lenSavedArray = contentLength;
                memcpy(body,ptr,contentLength);
                body[contentLength] = '\0';
                memcpy(savedArray,ptr,contentLength);
                savedArray[contentLength] = '\0';
                char header[40] = "Content-Length: ";
                sprintf(header + strlen(header), "%d", contentLength);
                strcat(headers,header);
                strcat(headers,"\r\n");
            }
            

        } else {
            responseType = 1;
        }
    } else if(strcmp(method, "OTHER") == 0) {
        //other method
    } else {
        //invalid method
        responseType = 1;
    }
    //Make the response here
    
    
    responseType:
    response[0] = '\0';
    
    if(responseType == 0) {
        strcat(response,version);
        strcat(response," 200 OK\r\n");
        strcat(response,headers);
        strcat(response,"\r\n");
        stats2->requests += 1;
        
        if(needMultipleSends == 1) {
            //sends the body later
            body[0] = '\0';
        }
        
    } else if(responseType == 1) {
        strcat(response,version);
        strcat(response," 400 Bad Request\r\n");
        strcat(response,"\r\n");
        strcat(response,"\0");
        int len_response = strlen(response);
        stats2->errors+=1;
        stats2->error_bytes+=len_response;
    } else if(responseType == 2) {
        strcat(response,version);
        strcat(response," 404 Not Found\r\n");
        strcat(response,"\r\n");
        strcat(response,"\0");
        int len_response = strlen(response);
        stats2->errors+=1;
        stats2->error_bytes+=len_response;
    } else if(responseType == 3) {
        strcat(response,version);
        strcat(response," 413 Request Entity Too Large\r\n");
        strcat(response,"\r\n");
        strcat(response,"\0");
        int len_response = strlen(response);
        stats2->errors+=1;
        stats2->error_bytes+=len_response;
    } else {
        //error
    }
    //the header byets also includes the bytes from the errors
    //Send the response
    int bytesRead;
    if(needMultipleSends == 0) {
        int len_response = strlen(response);
        Send(clientfd, response, len_response, 0);
        
        if(responseType == 0) {
            stats2->header_bytes += len_response;
            Send(clientfd, body, contentLength, 0);
            stats2->body_bytes += contentLength;
        }
        r1->headerSent = 1;
        r1->allSent = 1;
        
    } else {//only send header for common get request
        int len_response = strlen(response);
        Send(clientfd, response, strlen(response), 0);
        stats2->header_bytes += len_response;
        r1->headerSent = 1;
        r1->allSent = 0;
        (*sizeofFile) = contentLength;
        (*fileopen) = fd;


    }

    
    return;

}




static int open_listenfd(int port) {
    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);//SockStream is TCP, AF = Address Family
    static struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1",&(server.sin_addr));//run on local host

    // allow for reuse
    int optval = 1;//sets the options
    setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));

    //Binds socket address to listenfd
    Bind(listenfd, (struct sockaddr*)&server, sizeof(server));
    
    Listen(listenfd,10);
    return listenfd;

}
static int accept_client(int listenfd) {
    static struct sockaddr_in client;
    static socklen_t csize = 0;
    // make sure the client is empty, memset clears client
    memset(&client,0x00,sizeof(client));
    int clientfd = Accept(listenfd,(struct sockaddr*)&client, &csize);
    return clientfd;
}
int Epoll_Wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    int epoll_wait_ret = epoll_wait(epfd,events,maxevents,timeout);
    if(epoll_wait_ret < 0) {
        perror("epoll_wait");
        exit(1);
    }
    return epoll_wait_ret;
}
int Epoll_Ctl(int epfd, int op, int fd, struct epoll_event * ev) {
    int ecRet = epoll_ctl(epfd,op,fd,ev);
    if(ecRet < 0) {
        perror("epoll_ctl");
        exit(1);
    }
    return ecRet;
}



int main(int argc, char * argv[])
{
    int port = get_port();

    printf("Using port %d\n", port);
    printf("PID: %d\n", getpid());

    // Make server available on port
    // Opens listening socket
    int listenfd = open_listenfd(port);
    char savedArray[RMAX];
    savedArray[0] = '\0';
    int lenSavedArray = 0;
    struct Server_stats stats1;
    stats1.requests = 0;
    stats1.header_bytes = 0;
    stats1.body_bytes = 0;
    stats1.errors = 0;
    stats1.error_bytes = 0;
    
    //epoll - case 9/10
    int epfd = epoll_create1(0);//epoll instance
    static struct epoll_event Event;
    memset (&Event, 0x00, sizeof(Event));
    Event.events = EPOLLIN;
    Event.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &Event);
    
    struct epoll_event event_list[16];
    headOfArray = NULL;
    
    
    // Process client requests
    while (1) {
        //add epoll wait
        int nfds = Epoll_Wait(epfd,event_list,16,-1);
        //sets timeout = -1, waits forever for IO event
        
        //events:
        //1) accepting new client
        //2) client is ready for reading(has a request)
        //3) client is ready for writing(can send response)
        for(int i = 0; i < nfds; i++) {
            int fd = event_list[i].data.fd;
            if(event_list[i].events == EPOLLIN) {
                if(fd == listenfd) {
                    int newclient = accept_client(listenfd);
                    memset(&Event, 0x00, sizeof(Event));
                    Event.events = EPOLLIN;
                    Event.data.fd = newclient;
                    Epoll_Ctl(epfd, EPOLL_CTL_ADD, newclient, &Event);
                    //new client ready for connection
                    //accept client
                    //add the client to epoll instance
                } else {
                    //already accepted just
                    //recv the request, fulfill req
                    
                    RSIZE = Recv(fd, request, RMAX, 0);
                    
                    if(RSIZE == 0) {
                        //do nothing
                    } else {
                       
                        request[RSIZE] = '\0';
                        addRequest(RSIZE,fd,request,0,0,0,-1,0);
                        req1ArrayCount++;

                        memset(&Event, 0x00, sizeof(Event));
                        Event.events = EPOLLIN;
                        Event.data.fd = fd;
                        Epoll_Ctl(epfd,EPOLL_CTL_DEL,fd,&Event);
                        
                        //handle the header for common get requests and fulfill requests for other requests
                        int ffd1 = -1;
                        int sizeOfBody = 0;
                        int index = 0;
                        struct Req1* pointer = headOfArray;
                        
                        while(pointer != NULL) {//find correct request struct
                            if(pointer->fd == fd) {
                                index = pointer->index;
                                break;
                            }
                            pointer = pointer->next;
                        }
                        handle_request(fd,savedArray,&lenSavedArray,&stats1,index,&ffd1,&sizeOfBody);
                        pointer->fileopen = ffd1;
                        pointer->numberOfBytes = sizeOfBody;
                        if(pointer->allSent == 0) {
                            memset(&Event, 0x00, sizeof(Event));
                            Event.events = EPOLLOUT;
                            Event.data.fd = fd;
                            Epoll_Ctl(epfd, EPOLL_CTL_ADD, fd, &Event);
                        } else {
                            //all sent == 1
                            removeRequest(index);
                            close(fd);
                        }

                    }
                    
                    

                }
            } else {
                //EPOLLOUT
                //send 1000 bytes
                struct Req1 thereq;//has fd,request[], and rsize
                int index = 0;
                struct Req1* pointer = headOfArray;
                while(pointer != NULL) {
                    if(pointer->fd == fd) {
                        index = pointer->index;
                        break;
                    }
                    pointer = pointer->next;
                }
                
                
                
                if(pointer->allSent == 1) {
                    //supposed to remove from linked list
                    memset(&Event, 0x00, sizeof(Event));
                    Event.events = EPOLLOUT;
                    Event.data.fd = fd;
                    Epoll_Ctl(epfd,EPOLL_CTL_DEL,fd,&Event);
                    
                    close(fd);
                    close(pointer->fileopen);
                    removeRequest(index);
                } else {
                    //send_chunk
                    send_chunk(fd,pointer->fileopen,pointer->numberOfBytes,&stats1,index);
                    
                }
                
                
                

            }
        }

        
    }

    return 0;
}


static int get_port(void)
{
    int fd = open("port.txt", O_RDONLY);
    if (fd < 0) {
        perror("Could not open port.txt");
        exit(1);
    }

    char buffer[32];
    int r = read(fd, buffer, sizeof(buffer));
    if (r < 0) {
        perror("Could not read port.txt");
        exit(1);
    }

    return atoi(buffer);
}

