/* 
 * echoservers.c - A concurrent echo server based on select
 */
/* $begin echoserversmain */
#include "csapp.h"
// Dimensions for the drawn grid (should be GRIDSIZE * texture dimensions)
#define GRID_DRAW_WIDTH 640
#define GRID_DRAW_HEIGHT 640

#define WINDOW_WIDTH GRID_DRAW_WIDTH
#define WINDOW_HEIGHT (HEADER_HEIGHT + GRID_DRAW_HEIGHT)

// Header displays current score
#define HEADER_HEIGHT 50

// Number of cells vertically/horizontally in the grid
#define GRIDSIZE 10

typedef struct { /* represents a pool of connected descriptors */ //line:conc:echoservers:beginpool
    int maxfd;        /* largest descriptor in read_set */   
    fd_set read_set;  /* set of all active descriptors */
    fd_set ready_set; /* subset of descriptors ready for reading  */
    int nready;       /* number of ready descriptors from select */   
    int maxi;         /* highwater index into client array */
    int clientfd[FD_SETSIZE];    /* set of active descriptors */
    rio_t clientrio[FD_SETSIZE]; /* set of active read buffers */
} pool; //line:conc:echoservers:endpool
/* $end echoserversmain */
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p);
void moveTo(int x, int y);
void checkMovement(char move);
/* $begin echoserversmain */

int byte_cnt = 0; /* counts total bytes received by server */
typedef struct
{
    int x;
    int y;
} Position;

typedef enum
{
    TILE_GRASS,
    TILE_TOMATO
} TILETYPE;

TILETYPE grid[GRIDSIZE][GRIDSIZE];

Position playerPosition;
int score;
int level;
int numTomatoes;

double rand01()
{
    return (double) rand() / (double) RAND_MAX;
}

void initGrid()
{
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            double r = rand01();
            if (r < 0.1) {
                grid[i][j] = 'T';
                numTomatoes++;
            }
            else
                grid[i][j] = 'G';
        }
    }

    // force player's position to be grass
    if (grid[playerPosition.x][playerPosition.y] == 'T') {
        grid[playerPosition.x][playerPosition.y] = 'G';
        numTomatoes--;
    }
    // ensure grid isn't empty
    while (numTomatoes == 0)
        initGrid();

    grid[playerPosition.x][playerPosition.y] = 'P';
    for (int i = GRIDSIZE-1; i >=0; i--) {
        for (int j = 0; j < GRIDSIZE; j++){
		printf("%c",grid[j][i]);
	}
	printf("\n");
    }
}

int main(int argc, char **argv)
{
    int listenfd, connfd, port; 
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    static pool pool; 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    port = atoi(argv[1]);
    //grid[playerPosition.x][playerPosition.y] == 'P';
    initGrid();

    listenfd = Open_listenfd(port);
    init_pool(listenfd, &pool); //line:conc:echoservers:initpool
    while (1) {
	/* Wait for listening/connected descriptor(s) to become ready */
	pool.ready_set = pool.read_set;
	pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

	/* If listening descriptor ready, add new client to pool */
	if (FD_ISSET(listenfd, &pool.ready_set)) { //line:conc:echoservers:listenfdready
	    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:conc:echoservers:accept
	    add_client(connfd, &pool); //line:conc:echoservers:addclient
	}
	
	/* Echo a text line from each ready connected descriptor */ 
	check_clients(&pool); //line:conc:echoservers:checkclients
    }
}
/* $end echoserversmain */

/* $begin init_pool */
void init_pool(int listenfd, pool *p) 
{
    /* Initially, there are no connected descriptors */
    int i;
    p->maxi = -1;                   //line:conc:echoservers:beginempty
    for (i=0; i< FD_SETSIZE; i++)  
	p->clientfd[i] = -1;        //line:conc:echoservers:endempty

    /* Initially, listenfd is only member of select read set */
    p->maxfd = listenfd;            //line:conc:echoservers:begininit
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set); //line:conc:echoservers:endinit
}
/* $end init_pool */

/* $begin add_client */
void add_client(int connfd, pool *p) 
{
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
	if (p->clientfd[i] < 0) { 
	    /* Add connected descriptor to the pool */
	    p->clientfd[i] = connfd;                 //line:conc:echoservers:beginaddclient
	    Rio_readinitb(&p->clientrio[i], connfd); //line:conc:echoservers:endaddclient

	    /* Add the descriptor to descriptor set */
	    FD_SET(connfd, &p->read_set); //line:conc:echoservers:addconnfd

	    /* Update max descriptor and pool highwater mark */
	    if (connfd > p->maxfd) //line:conc:echoservers:beginmaxfd
		p->maxfd = connfd; //line:conc:echoservers:endmaxfd
	    if (i > p->maxi)       //line:conc:echoservers:beginmaxi
		p->maxi = i;       //line:conc:echoservers:endmaxi
	    break;
	}
    if (i == FD_SETSIZE) /* Couldn't find an empty slot */
	app_error("add_client error: Too many clients");
}
/* $end add_client */

/* $begin check_clients */
void check_clients(pool *p) 
{
    int i, connfd, n;
    char buf[MAXLINE]; 
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
	connfd = p->clientfd[i];
	rio = p->clientrio[i];

	/* If the descriptor is ready, echo a text line from it */
	if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) { 
	    p->nready--;
	    if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
		byte_cnt += n; //line:conc:echoservers:beginecho
		printf("Server received %d (%d total) bytes on fd %d\n", 
		       n, byte_cnt, connfd);

		Rio_writen(connfd, buf, n); //line:conc:echoservers:endecho
		checkMovement(buf[0]);
		printf("%s\n",buf);
	for (int i = GRIDSIZE-1; i >=0; i--) {
        	for (int j = 0; j < GRIDSIZE; j++){
                	printf("%c",grid[j][i]);
        	}
		printf("\n");
	}
	    }

	    /* EOF detected, remove descriptor from pool */
	    else { 
		Close(connfd); //line:conc:echoservers:closeconnfd
		FD_CLR(connfd, &p->read_set); //line:conc:echoservers:beginremove
		p->clientfd[i] = -1;          //line:conc:echoservers:endremove
	    }
	}
    }
}
/* $end check_clients */

void moveTo(int x, int y)
{
	printf("i tried to move\n");
    // Prevent falling off the grid
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return;

    // Sanity check: player can only move to 4 adjacent squares
    if (!(abs(playerPosition.x - x) == 1 && abs(playerPosition.y - y) == 0) &&
        !(abs(playerPosition.x - x) == 0 && abs(playerPosition.y - y) == 1)) {
        //fprintf(stderr, "Invalid move attempted from (%d, %d) to (%d, %d)\n", playerPosition.x, playerPosition.y, x, y);
        printf("Mistake here\n");
	return;
    }
    grid[playerPosition.x][playerPosition.y]='G';
    playerPosition.x = x;
    playerPosition.y = y;   
    grid[playerPosition.x][playerPosition.y]='P';
    if (grid[x][y] == TILE_TOMATO) {
        grid[x][y] = TILE_GRASS;
        score++;
        numTomatoes--;
        if (numTomatoes == 0) {
            level++;
            initGrid();
        }
    

}
}
void checkMovement(char move){
	if(move=='w'){
		moveTo(playerPosition.x,playerPosition.y+1);
		printf("I moved up");
		printf("The x is: %d and the y is %d\n",playerPosition.x,playerPosition.y);
	}
	if(move=='a'){
		moveTo(playerPosition.x-1,playerPosition.y);
		printf("I moved left");
		printf("The x is: %d and the y is %d\n",playerPosition.x,playerPosition.y);
	}
	if(move=='s'){
		moveTo(playerPosition.x,playerPosition.y-1);
		printf("I moved down");
		printf("The x is: %d and the y is %d\n",playerPosition.x,playerPosition.y);
	}
	if(move=='d'){
		moveTo(playerPosition.x+1,playerPosition.y);
		printf("I moved right");
		printf("The x is: %d and the y is %d\n",playerPosition.x,playerPosition.y);
	}
}

