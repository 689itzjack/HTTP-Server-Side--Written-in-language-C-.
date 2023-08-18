README
Author: Ytzhak Ariel Armoni 

HTTP Server Side

===Description ===

- Constructs an HTTP response based on client's request.
- Sends the response to the client.

When using TCP, a server creates a socket for each client it talks to. In other words, there is always
one socket where the server listens to connections and for each client connection request,
the server opens another socket. In order to enable multithreaded program, the server
should create threads that handle the connections with the clients. Since, the server
should maintain a limited number of threads, it constructs a thread pool. In other words,
the server creates the pool of threads in advanced and each time it needs a thread to
handle a client connection, it take one from the pool or enqueue the request if there is no
available thread in the pool.

Program files:
server.c- file that implementaion of a server and its Actions.


Funcation:

int printErrorsServer(int port,int poolsize,int typeError){//0 wrong command, 1 System Call Failed 

char *get_mime_type(char *name){//we call this function when we know that the carpet and the file they have read permisssions and we know
// that the files name is legal and exists and must to be what the parameter recives a file and non a carpet 
//the result from the function we conect it to the  header "contents type "

char* checkHTMLinPath(char *originalPath){//return the string of the path if the file html that we searching it exist and - if not exist

int sizefiles(char *comm){ //this function returns the size of the line of arrfiles array

char **arrfiles(char *comm){ //this function returns an array of the number of files in the request

char* request_Client(char* path){//this mehod gets the first line of the client request 

int sizeToken(char *comm){ //this function returns the size of the line of tokens array

char** requestClientLegal(char* request){//this method cut the client reuest in 3 token and return an array two dimentional
                                        //if we get 3 tokens other wise char* makingTable(char *originalPath){    //return the string of the path if the file html that we searching it exist and -1 if not exist

int checkingPermissionsPath(char** path){// if return -1 so the path doesnt exist, if return 0 the path have all the permissions,if 1 no permissions

char* responseHTML_with_Base(int typeRequest,char* path){//this fuction returns the body of the file or error

char* responseBase(int typeRequest,char* files){//the method recives the type of request and pathe, return the headera ith the body to the caller function

int func_Response_to_Client(void* arg){// this function returns the answer to the client and close conection 

threadpool.c - makes the function of a parallel working of the server

functions:

/**
 * create_threadpool creates a fixed-sized thread
 * pool.  If the function succeeds, it returns a (non-NULL)
 * "threadpool", else it returns NULL.
 * this function should:
 * 1. input sanity check 
 * 2. initialize the threadpool structure
 * 3. initialized mutex and conditional variables
 * 4. create the threads, the thread init function is do_work and its argument is the initialized threadpool. 
 */
threadpool* create_threadpool(int num_threads_in_pool);

/**
 * dispatch enter a "job" of type work_t into the queue.
 * when an available thread takes a job from the queue, it will
 * call the function "dispatch_to_here" with argument "arg".
 * this function should:
 * 1. create and init work_t element
 * 2. lock the mutex
 * 3. add the work_t element to the queue
 * 4. unlock mutex
 *
 */
void dispatch(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg);

/**
 * The work function of the thread
 * this function should:
 * 1. lock mutex
 * 2. if the queue is empty, wait
 * 3. take the first element from the queue (work_t)
 * 4. unlock mutex
 * 5. call the thread routine
 *
 */
void* do_work(void* p);

/**
 * destroy_threadpool kills the threadpool, causing
 * all threads in it to commit suicide, and then
 * frees all the memory associated with the threadpool.
 */
void destroy_threadpool(threadpool* destroyme);
