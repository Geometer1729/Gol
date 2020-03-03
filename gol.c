#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

void showByte(int8_t n){
	// Modified from https://stackoverflow.com/questions/1024389/print-an-int-in-binary-representation-using-c
	int8_t m=n;
	for(int i=0;i<8;i++){
		if(m&1){
			printf("@");
		}else{
			printf(" ");
		}
		m >>=1;
	}
}

void lineBreak(int x,char c){
		for(int a=0;a<9*x-1;a++)
			printf("%c",c);
		printf("\n");
}

void showBigGrid(int x,int y,uint64_t grid[x][y]){
	uint8_t * p;
	for(int j=0;j<y;j++){
		for(int row=0;row<8;row++){
			for(int i=x-1;i>=0;i--){
				p= (uint8_t *)&grid[i][j];
				showByte(*(p+row));
				//these lines are usefull when debuging boundry errors
				//printf("|");
			}
			printf("\n");
		}
		//lineBreak(x,'a');
	}
}

void showIntGrid(uint64_t  n){
    uint8_t * p = (uint8_t *)&n;
		for (int i=0;i<8;i++){
			showByte(*(p+i));
			printf("\n");
		}
}

void showLong(uint64_t n){
	for(int i=0;i<64;i++){ // modified from same stackoverflow answer
		if(n&1){
			printf("1");
		}else{
			printf("0");
		}
		n >>=1;
		printf(" ");
	}
	printf("\n");
}

inline uint64_t downRow(uint64_t i,uint64_t u){
	//the first byte comes is the last byte of u the next 7 bytes are the first 7 of i
	return (i >> 8) | (u << 56);
}

inline uint64_t upRow(uint64_t i,uint64_t d){
	return (i << 8) | (d >> 56);
}

inline uint64_t leftCol(uint64_t i,uint64_t r){
	//the 7 left bits of each byte come from shifting i left 1 and the last bit from shifting r right 7
	return ( (i << 1) & 0xfefefefefefefefe ) | ( ((r >> 7) & 0x0101010101010101));
}

inline uint64_t rightCol(uint64_t i,uint64_t l){
	return ( (i >> 1) & 0x7f7f7f7f7f7f7f7f ) | ( ((l << 7) & 0x8080808080808080));
}

//all these functions treat the grid as a torus and use the above 4 functions to shift it in one direction
void gridDown(int x,int y,uint64_t grid[x][y],uint64_t newGrid[x][y]){
	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			newGrid[i][j]=downRow(grid[i][j],grid[i][(j+1)%y]);
		}
	}
}

void gridUp(int x,int y,uint64_t grid[x][y],uint64_t newGrid[x][y]){
	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			newGrid[i][j]=upRow(grid[i][j],grid[i][(y+j-1)%y]);
		}
	}
}

void gridLeft(int x,int y,uint64_t grid[x][y] ,uint64_t newGrid[x][y]){
	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			newGrid[i][j]=leftCol(grid[i][j],grid[(i+1)%x][j]);
		}
	}
}

void gridRight(int x,int y,uint64_t grid[x][y],uint64_t newGrid[x][y]){
	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			newGrid[i][j]=rightCol(grid[i][j],grid[(x+i-1)%x][j]);
		}
	}
}

void golStep(int x,int y,uint64_t old[x][y],uint64_t new[x][y]){
	uint64_t shifts [9][x][y];
	// I think there is a way to avoid coppying this but I'm not sure how
	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			shifts[0][i][j]=old[i][j];
		}
	}
	//left and right shifts are done first so they are done fewer times because I think they are a bit slower
	gridLeft(x,y,shifts[0],shifts[1]);
	gridRight(x,y,shifts[0],shifts[2]);
	for(int i=0;i<3;i++){
		gridUp  (x,y,shifts[i],shifts[i+3]); 
		gridDown(x,y,shifts[i],shifts[i+6]); 
	}
	// d stores the bits of the count, c stores the carrys
	// a 3 bit count is good enough as knowing rather a cell had 0 or 8 neighbors is never nesecary
	uint64_t d[3][x][y];
	uint64_t c[2][x][y];

	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			// first few are done outside a loop because they can be done more efficently when some vars are known to be 0
			d[0][i][j]=shifts[1][i][j]^shifts[2][i][j];
			d[1][i][j]=shifts[1][i][j]&shifts[2][i][j];
			c[0][i][j]=d[0][i][j]&shifts[3][i][j];
			d[0][i][j]^=shifts[3][i][j];
			d[2][i][j]=c[0][i][j]&d[1][i][j];
			d[1][i][j]^=c[0][i][j];
			for(int n=4;n<9;n++){
				c[0][i][j]=d[0][i][j]&shifts[n][i][j];
				d[0][i][j]^=shifts[n][i][j];
				c[1][i][j]=d[1][i][j]&c[0][i][j];
				d[1][i][j]^=c[0][i][j];
				d[2][i][j]^=c[1][i][j];
			}
		}
	}

	for(int i=0;i<x;i++){
		for(int j=0;j<y;j++){
			// alive and _01_ (2 or 3) or dead and _010 (exactly 2)
			new[i][j]=(old[i][j]&(~d[2][i][j])&d[1][i][j]) | (~d[2][i][j]&d[1][i][j]&d[0][i][j]);
		}
	}
	return;
}

// atleast untill I add a way to read files these are handy for coding tests
uint64_t glider  = 0x0000000000020107;
uint64_t glider2 = 0xc0a0800000000000;
uint64_t spiner  = 0x0000000070000000;
uint64_t edge    = 0x010101010101;

int main(){
		uint64_t x,y;
		x=4;
		y=3;
		uint64_t grid [x][y];
		uint64_t newGrid[x][y];

		for(int i=0;i<x;i++){
			for(int j=0;j<y;j++){
				grid[i][j]=0;
			}
		}
		grid[0][1]=glider;
		showBigGrid(x,y,grid);
		lineBreak(x,'#');
		for(int i=0;i<100000000;i++){
			golStep(x,y,grid,newGrid);
			golStep(x,y,newGrid,grid);
		}
		showBigGrid(x,y,grid);
}
