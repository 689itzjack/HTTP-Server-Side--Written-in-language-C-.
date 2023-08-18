#include "threadpool.h"
#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>        
#include <unistd.h>        	          // for read/write/close
#include <sys/types.h>     	          /* standard system types*/
#include <netinet/in.h>    	          /* Internet address structures */
#include <sys/socket.h>   	          /* socket interface functions  */
#include <netdb.h>         	          /* host to IP resolution*/
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#define h_addr h_addr_list[0] /* for backward compatibility */
#define  BUFLEN  1000          /* maximum response size     */
#define SIZEREQUEST 5000         //
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"


/////////////////////////////////////////PART OF THE FUNCTIONS////////////////////////////////////////////////////////////////
void printErrorsServer(){//0 wrong command, 1 System Call Failed 
		printf("Usage: server <port> <pool-size>\n");
        exit(EXIT_FAILURE);
}


char *get_mime_type(char *name){//we call this function when we know that the carpet and the file they have read permisssions and we know
// that the files name is legal and exists and must to be what the parameter recives a file and non a carpet 
//the result from the function we conect it to the  header "contents type "
    char *ext = strrchr(name, '.');
    if (!ext) return NULL;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".au") == 0) return "audio/basic";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0) return "video/mpeg";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
char* checkHTMLinPath(char *originalPath){//return the string of the path if the file html that we searching it exist and - if not exist
                                            
    struct dirent *dentry;
    char* returned;
    char buffer[100];
    buffer[0]='\0';
   
    if(originalPath[0] == '/' && strcmp(originalPath,"/")!=0)// 
        memmove(originalPath, originalPath+1, strlen(originalPath));
    if(strcmp(originalPath,"/")==0){
        strcat(buffer,"./");
    }    
    
    strcat(buffer,originalPath);
    DIR *dir=opendir(buffer);
    if (dir == NULL){
        return NULL;
    }
    while ((dentry = readdir(dir)) != NULL){
        if(strstr(dentry->d_name,".html")){
            strcat(buffer,dentry->d_name);
            strcat(buffer,"\0");
            returned=buffer;
            closedir(dir);
            return returned;
        }
    }
    returned="-";
    closedir(dir);
    return returned;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
int sizefiles(char *comm){ //this function returns the size of th of arrfiles array
    int count = 1;
    char temp[BUFLEN];
    temp[0] = '\0';
    strcpy(temp, comm);
    char *token = strtok(temp, "/");
    while (token != NULL)
    {
        count++;
        token = strtok(NULL, "/");
    }
    return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char **arrfiles(char *comm){ //this function returns an array of the number of files in the request


    if(strcmp(comm,"/")==0){
        char **arrFiles = (char **)malloc(2 * sizeof(char *));
        arrFiles[0]="/";
        arrFiles[1]=NULL;
        return arrFiles;
    }
        
    char temp[BUFLEN];
    temp[0] = '\0';
    strcpy(temp, comm);
    int size = sizefiles(comm);
    char **arrFiles = (char **)malloc(size * sizeof(char *));
    if (arrFiles == NULL)
    { //if we return null so we send to client internal server error
        perror("Error: malloc filed");
        return NULL;
    }
    char *token = strtok(temp, "/");
    int i = 0;
    while (token != NULL)
    {
        arrFiles[i] = token;
        i++;
        token = strtok(NULL, "/");
    }

    arrFiles[i]=token;
    return arrFiles;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* request_Client(char* path){//this mehod gets the firs of the client request 
    
    char *token = strtok(path, "\r\n");
    return token;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int sizeToken(char *comm){ //this function returns the size of th of tokens array
    int count = 1;
    char temp[BUFLEN];
    strcpy(temp,comm);
    char *token = strtok(temp, " ");
    while (token != NULL)
    {
        count++;
        token = strtok(NULL, " ");
    }
    return count;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char** requestClientLegal(char* request){//this method cut the client reuest in 3 token and return an array two dimentional
                                        //if we get 3 tokens other wise null
    int num_tokens=0;
    int index=0;
    char copied[BUFLEN];
    copied[0]='\0';
    char copied2[BUFLEN];
    copied2[0]='\0';
    strcpy(copied,request);
    strcpy(copied2,request);
    int a=sizeToken(copied2);
    char** newpath = (char**) malloc(a*sizeof(char*));
     if(newpath==NULL){
        perror("ERROR memory allocation\n");
        return NULL;
    } 
    for(int in=0;in<a;in++){
        newpath[in]=NULL;
    }
    char *token = strtok(copied," ");

    while (token != NULL)
    {
        newpath[index]=token;
        index++;
        num_tokens++;
        token = strtok(NULL," ");
    }
    return newpath;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* makingTable(char *originalPath){    //return the string of the path if the file html that we searching it exist and -1 if not exist
                                            
    struct stat fs;
    struct dirent *dentry;
    char* returned;
    char buffer[50];
    buffer[0]='\0';
    char table[2000];
    table[0]='\0';

    if (originalPath[0] == '/' && strcmp(originalPath,"/")!=0) 
        memmove(originalPath, originalPath+1, strlen(originalPath));//remove the char from up
    if(strcmp(originalPath,"/")==0){
        strcat(buffer,".");
    }
    strcat(buffer,originalPath);
    time_t last_mod;
    char timebuf[128];
    timebuf[0]='\0';
    DIR *dir=opendir(buffer);
    if (dir == NULL){
        return NULL;
    }
    while ((dentry = readdir(dir)) != NULL){
        strcat(buffer,dentry->d_name);
        strcat(table,"<tr>\r\n<td><A HREF=\"");
        strcat(table,dentry->d_name);
        strcat(table,"\">");
        strcat(table,dentry->d_name);
        strcat(table,"</A></td><td>");
        if(stat(buffer,&fs)==-1){
            return NULL;
        }
        last_mod=fs.st_mtime;
        strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&last_mod));
        strcat(table,timebuf);
        strcat(table,"</td>\r\n<td>");
        if (S_ISREG(fs.st_mode)){
            char sizeHtmlString[100];
            sprintf(sizeHtmlString, "%ld", fs.st_size);
            strcat(table,sizeHtmlString);
        }
        strcat(table,"</td>\r\n</tr>\r\n");
        buffer[0]='\0';
        if(strcmp(originalPath,"/")!=0)
            strcat(buffer,originalPath);
    }
    returned=table;
    closedir(dir);
    return returned;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int checkingPermissionsPath(char** path){// if return -1 so the path doesnt exist, if return 0 the path have all the permissions,if 1 no permissions
    int flag=0;
    struct stat fs;
    char searchInPath[SIZEREQUEST];
    searchInPath[0] ='\0';
    int i=0;
    strcat(searchInPath, path[i]);
    
    while(path[i]!=NULL){

        if (stat(searchInPath, &fs) == -1) //return -1 if the file doesnt exist
            return 5;

        if (S_ISDIR(fs.st_mode)){//THIS PART MEANS THAT ITS A CARPET
            if(strchr(path[i],'.')==NULL){
                strcat(searchInPath, "/");
            }
            if ( (S_IXOTH & fs.st_mode) || (S_IROTH & fs.st_mode)) {//CHECK THE PERMISSIONS OF THE CARPET
                flag=0;
                i++;
                if(path[i]==NULL)
                    return flag;
                strcat(searchInPath, path[i]);
                /*
                if(strchr(path[i],'.')==NULL)
                    strcat(searchInPath, "/");*/
                  
                continue;
            }
            else{
                flag=1;//the folder has not permission
                break;
            }
        }

        if (S_ISREG(fs.st_mode)){

            if ( S_IROTH & fs.st_mode) {
                if(strchr(searchInPath,'.')==NULL){
                    flag=2;
                    i++;
                    continue;
                }
                flag=0;
                i++;
                continue;
            }
            else{
                flag=1;//the file has not permission
                break;
            }
        }

        if(!(S_ISREG(fs.st_mode))){
            flag=1;
            return flag;
        } 
    }
    return flag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* responseHTML_with_Base(int typeRequest,char* path){//this fuction returns the body of the file or error
    char* send;
    char html[SIZEREQUEST];
    html[0]='\0';
    
    if(typeRequest==0){
        strcat(html,"<HTML><HEAD><TITLE>400 Bad Request</TITLE></HEAD>\r\n<BODY><H4>400 Bad request</H4>\r\nBad Request.\r\n</BODY></HTML>\r\n");
    }
    if(typeRequest==1){
        strcat(html,"<HTML><HEAD><TITLE>501 Not supported</TITLE></HEAD>\r\n<BODY><H4>501 Not supported</H4>\r\nMethod is not supported.\r\n</BODY></HTML>\r\n");
    }
    if(typeRequest==2){
        strcat(html,"<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\r\n<BODY><H4>404 Not Found</H4>\r\nFile not found.\r\n</BODY></HTML>\r\n");
    }
    if(typeRequest==3){
         strcat(html,"<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>\r\n<BODY><H4>403 Forbidden</H4>\r\nAccess denied.\r\n</BODY></HTML>\r\n");
    }
    if(typeRequest==4){
        strcat(html,"<HTML><HEAD><TITLE>302 Found</TITLE></HEAD>\r\n<BODY><H4>302 Found</H4>\r\nDirectories must end with a slash.\r\n</BODY></HTML>\r\n");
    }
    if(typeRequest==5){
        strcat(html,"<HTML>\r\n<HEAD><TITLE>Index of ");
        strcat(html,path);
        strcat(html,"</TITLE></HEAD>\r\n\r\n<BODY>\r\n<H4>Index of ");
        strcat(html,path);
        strcat(html,"</H4>\r\n\r\n<table CELLSPACING=8>\r\n<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\r\n");
        char* table=makingTable(path);
        strcat(html,table);
        strcat(html,"</table>\r\n<HR>\r\n<ADDRESS>webserver/1.0</ADDRESS>\r\n</BODY></HTML>\r\n");
    }
    if(typeRequest==9){
        strcat(html,"<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\r\n<BODY><H4>500 Internal Server Error</H4>\r\nSome server side error.\r\n</BODY></HTML>\r\n");

    }
    send=html;
    return send;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* responseBase(int typeRequest,char* files){//the method recives the type of request and pathe, return the anser to the client
    char* send;// final answer to the client
    char* html;
    char response[SIZEREQUEST];    
    response[0]='\0';
    struct stat fs ;
    time_t last_mod;
    time_t now;// format of the hours
    char timebuf[128];
    now = time(NULL);
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    char filesCopy[BUFLEN];
    if(files!=NULL){
        
        for(int i=0;i<BUFLEN;i++){
            filesCopy[0]='\0';
        }
        strcpy(filesCopy,files);
    }
    
    int sizeHTML;
    if(typeRequest!=6 && typeRequest!=7 && typeRequest!=8){//make the body html for an error
        html=responseHTML_with_Base(typeRequest,files);//here we can the file html according to the type of file or error
        sizeHTML=strlen(html);
    }
    char sizeHtmlString[500];
    sprintf(sizeHtmlString, "%d", sizeHTML);

    strcat(response,"HTTP/1.1 ");

    if(typeRequest==0){//if the path cpontains more than 3 tokens or the protocols are not HTTP/1.1 or 1.0
        strcat(response,"400 Bad Request");//error in path
    }
    if(typeRequest==1){
        strcat(response,"501 Not supported");//file not supported
    }
    if(typeRequest==2){
        strcat(response,"404 Not Found");//file not founded or not exist
    }
    if(typeRequest==3){
        strcat(response,"403 Forbidden");//the file has not permissions
    }
    if(typeRequest==4){
        strcat(response,"302 Found");//redirection
    }
    if(typeRequest==5 || typeRequest==6 || typeRequest==7 || typeRequest==8){//conected
        strcat(response,"200 OK");
    }
    if(typeRequest==9){
        strcat(response,"500 Internal Server Error");//error in conection
    }

    strcat(response,"\r\nServer: webserver/1.0\r\nDate: ");
    strcat(response,timebuf);

    if(typeRequest==4){//if e got redirection, it means error 302
        char temp1[strlen(files)+2];
        temp1[0]='\0';
        strcat(temp1,files);
        strcat(temp1,"/");
        strcat(response,"\r\nLocation: ");
        strcat(response,temp1);
    }
    if(typeRequest!=7 && typeRequest!=6 && typeRequest!=8){//all the requests that they are html 
        strcat(response,"\r\nContent-Type: text/html\r\nContent-Length: ");
        strcat(response,sizeHtmlString);
    }

    if (typeRequest==7 || typeRequest==6 || typeRequest==8){
        int rc;
        if(typeRequest==7 || typeRequest==8){
            if(typeRequest!=8){
                strcat(response,"\r\nContent-Type: ");
                strcat(response,get_mime_type(files)); 
            }   
            strcat(response,"\r\nContent-Length: ");
            if (files[0] == '/')
            memmove(files, files+1, strlen(files));
            int fd=open(files,O_RDONLY,0);
            struct stat fs ;
            if (stat(files, &fs) == -1)
                return NULL;
            rc=fs.st_size;
            sprintf(sizeHtmlString, "%d", rc);
            strcat(response,sizeHtmlString);
            close(fd);
        }
        else{
            
            strcat(response,"\r\nContent-Type: text/html\r\nContent-Length: ");
            struct stat fs ;
            char *fileHTML2=checkHTMLinPath(filesCopy);
            if (stat(fileHTML2, &fs) == -1)
                return NULL;
            rc=fs.st_size;
            sprintf(sizeHtmlString, "%d", rc);
            strcat(response,sizeHtmlString);
        }
    }
    if (typeRequest==5 || typeRequest==6 || typeRequest==7 || typeRequest==8){
        strcat(response,"\r\nLast-Modified: ");
        stat(files, &fs);
        last_mod=fs.st_mtime;
        strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&last_mod));
        strcat(response,timebuf);
    }
    strcat(response,"\r\nConnection: close\r\n\r\n");
    if(typeRequest!=6 && typeRequest!=7 && typeRequest!=8){
        strcat(response,html);
    }
    send=response;
    return send;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int func_Response_to_Client(void* arg){// this function returns the answer to the client and close conection 
    char bufer[SIZEREQUEST];
    bufer[0]='\0';
    int newSOCKET=*(int*)arg;
    int flag=0;
    char *finalAnswer;
    int counter=0;
    char** files;
    char** legalreq;

    read(newSOCKET,bufer,sizeof(bufer)); //from client
    char* requestClient=request_Client(bufer);//request from the client, cut until the first line only

    legalreq=requestClientLegal(requestClient);//we obtain in this variable the 3 tokens from the client request and
                                            //null if we had an error.
    
    if(legalreq!=NULL){

        files=arrfiles(legalreq[1]);//this array contains just the path that contains the 3 tokens, every file in one index
                                    //null if occured an error of malloc  
        int resultPermissionPath=checkingPermissionsPath(files);
        
        if(files!=NULL){
            while(legalreq[counter]!=NULL){//??????????????????????????????????????
                counter++;
            }

            if(counter > 3 || counter < 3){//400 BAD REQUEST
                flag=0;
                finalAnswer=responseBase(flag,NULL);
            }

            else if(strcmp(legalreq[2],"HTTP/1.1")!=0 && strcmp(legalreq[2],"HTTP/1.0")!=0){//400 BAD REQUEST
                flag=0;
                finalAnswer=responseBase(flag,NULL);
            }

            else if(strcmp(legalreq[0],"GET")!=0){//501 not supported
                flag=1;
                finalAnswer=responseBase(flag,NULL);
            }
            
            else if(resultPermissionPath==5){// 404 Not Found, cheki---Per()
                flag=2;
                finalAnswer=responseBase(flag,NULL);
            }
            else if(resultPermissionPath==1){// there is not permissions, 403 Forbidden
                flag=3;
                finalAnswer=responseBase(flag,NULL);

            }
            else if(resultPermissionPath==0 || resultPermissionPath==2){// there are permissions completes, redirection
                char* str=legalreq[1];//the path of the file
                char temp=str[strlen(str)-1];
                if(strchr(str,'.')==NULL && temp!='/'){
                    if(resultPermissionPath==0){//this build the 302 file not found
                        flag=4;
                        finalAnswer=responseBase(flag,legalreq[1]);
                    }
                    else{
                        flag=8;
                        finalAnswer=responseBase(flag,legalreq[1]);
                    }
                    
                }
                else if(strchr(str,'.')==NULL && temp=='/'){//the path is legal(the par its legal and is folder)
                    char* checked=checkHTMLinPath(legalreq[1]);//check if the folder contins an html file       
                    if(strcmp(checked,"-")==0){// print the table of the folder(no exist file html)
                        flag=5;
                        finalAnswer=responseBase(flag,legalreq[1]);
                    }
                    else{//yes contains afile html
                        flag=6;
                        finalAnswer=responseBase(flag,legalreq[1]);
                    }
                }
                else if(strchr(str,'.')!=NULL){ // the path is a file
                    char* typeFile=get_mime_type(legalreq[1]);
                    if(typeFile==NULL){ //the file its not supported in our server
                        flag=8;
                        finalAnswer=responseBase(flag,legalreq[1]);
                    } 
                    else{//the file its supported in our server
                        flag=7;
                        finalAnswer=responseBase(flag,legalreq[1]);   
                    }  
                }
            }
        }
        else{
            flag=9;
            finalAnswer=responseBase(flag,NULL);
        }
    }
    else{
        flag=9;
        finalAnswer=responseBase(flag,NULL);
    }
    
    if(flag!=6 && flag!=7 && flag!=8)
        write(newSOCKET, finalAnswer, strlen(finalAnswer));//writting answer to client
    else{
        write(newSOCKET, finalAnswer, strlen(finalAnswer));//writting anwser to client()

        int rc;
        char* path=legalreq[1];
        if(flag==6){
            char temp[BUFLEN];
            temp[0]='\0';
            strcpy(temp,checkHTMLinPath(legalreq[1]));
            path=temp;
        }
        int fd=open(path,O_RDONLY,0);
        struct stat fs ;
        off_t file_size;
        if (stat(path, &fs) == -1)
            printf("WE HAVE A PROBLEM TO OPEN THE FILE!!!\n");
        file_size=fs.st_size;
        char file[file_size];
        rc = read(fd, file,file_size);
        write(newSOCKET, file, rc);//writting anwser to client()
        close(fd);
    }    
    requestClient=NULL;
    finalAnswer=NULL;
    bufer[0]='\0';
    free(legalreq);
    legalreq=NULL;
    free(files);
    files=NULL;
    close(newSOCKET);//conection with client
    return 0;
}

////////////////////////////////////////////MAINMAINMAINMAINMAINMAINMAINMAIN///////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
/////////////////////////////////////////////////INITIALIZATIONS SERVER/////////////////////////////////////////////////
struct sockaddr_in srv;	
if(argv[1]==NULL || argv[2]==NULL || argv[3]==NULL)
    printErrorsServer();

int port=atoi(argv[1]);
int numOfThreads=atoi(argv[2]);
int number_of_Tasks=atoi(argv[3]);

if(port < 0 || numOfThreads <= 0 || number_of_Tasks <= 0)
    printErrorsServer();
/*
int port=2001;
int numOfThreads=2;
int number_of_Tasks=5;
*/
int newSocket;
threadpool* newThreadpool=create_threadpool(numOfThreads);
int* arrayTasks=(int*) malloc(number_of_Tasks*sizeof(int));
int fd;		// WELCOME socket descriptor 
if((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: System Call socket() Failed !!!\n");
    exit(EXIT_FAILURE);
}
srv.sin_family = AF_INET; 
srv.sin_port = htons(port); 
srv.sin_addr.s_addr = htonl(INADDR_ANY);

if(bind(fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {//this is the initialization bind()
    perror("Error: System Call bind() Failed !!!\n");
    exit(EXIT_FAILURE);
}
if(listen(fd, 5) < 0) {
	perror("Error: System Call listen() Failed !!!\n");
	exit(EXIT_FAILURE);
}

int i;
for(i=0;i<number_of_Tasks;i++){
    newSocket = accept(fd,NULL, NULL);
    if(newSocket < 0) {
	    perror("Error: System Call accept() Failed !!!\n");	
        exit(EXIT_FAILURE);
    }

    arrayTasks[i]=newSocket;
    dispatch(newThreadpool,func_Response_to_Client,&arrayTasks[i]);
    
}
destroy_threadpool(newThreadpool);
close(fd);
free(arrayTasks);
arrayTasks=NULL;
    return 0;
}
