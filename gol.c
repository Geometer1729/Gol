#include <stdio.h>
#include <stdint.h>

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

void showGrid(uint64_t  n){
    uint8_t * p = (uint8_t *)&n;
		for (int i=0;i<8;i++){
			showByte(*(p+i));
			printf("\n");
		}
}

void showLong(uint64_t n){
	for(int i=0;i<64;i++){
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

uint64_t downRow(uint64_t i){
	return (i >> 8) | (i << 56);
}

uint64_t upRow(uint64_t i){
	return (i << 8) | (i >> 56);
}

uint64_t leftCol(uint64_t i){
	return ( (i << 1) & 0xfefefefefefefefe ) | ( ((i >> 7) & 0x0101010101010101));
}

uint64_t rightCol(uint64_t i){
	return ( (i >> 1) & 0x7f7f7f7f7f7f7f7f ) | ( ((i << 7) & 0x8080808080808080));
}

uint64_t golStep(uint64_t c){
	uint64_t shifts [9];
	shifts[0]=c;
	shifts[1]=leftCol(c);
	shifts[2]=rightCol(c);
	for(int i=0;i<3;i++){
		shifts[i+3]=downRow(shifts[i]);
		shifts[i+6]=upRow(shifts[i]);
	}
	uint64_t d1,d2,d3,c1,c2;
	d1=shifts[1]^shifts[2];
	d2=shifts[1]&shifts[2];
	c1=d1&shifts[3];
	d1^=shifts[3];
	d3=c1&d2;
	d2^=c1;
	// first few are done outside a loop because they are slightly simpler when you know they started as 0
	for(int i=4;i<9;i++){
		c1=d1&shifts[i];
		d1^=shifts[i];
		c2=d2&c1;
		d2^=c1;
		d3^=c2;
	}
	return (c&(~d3)&d2) | (~d3&d2&d1);
}

uint64_t glider = 0x0000000020107000;
uint64_t spiner = 0x0000000070000000;

int main(){
		uint64_t grid [8][8];
		for(int i=0;i<8;i++){
			for(int j=0;j<8;j++){
				grid[i][j]=0;
			}
		}
		grid[2][4]=glider;
}

