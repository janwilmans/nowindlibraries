#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"

#include "Zexall.h"
#include <iostream>

#define gotest(test_var) start = SDL_GetTicks(); test_var = 0; for (teller=0;teller<100000000;++teller) { test_var += 6; test_var += 16; test_var += 3; test_var += 8; test_var += 5; test_var += 11; } end = SDL_GetTicks(); cerr << " count: " << end-start << endl; 
 	
using namespace std;

int main(int argc, char *argv[])
{

	if( SDL_Init(SDL_INIT_TIMER ) <0 ) {
		printf("Unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}

	cerr << "Started..." << endl;
/*
	long start;
	long end;
	long long teller = 0;
 
 	long long test1;
	cerr << "long long "; gotest(test1)
	
 	long test2;
	cerr << "long "; gotest(test2)
	
 	int test3;
	cerr << "int "; gotest(test3)

	unsigned int test4;
	cerr << "unsigned int "; gotest(test4)

	unsigned long test5;
	cerr << "unsigned long "; gotest(test5)
	
	double test6;
	cerr << "double "; gotest(test6)
	
	float test7;
	cerr << "float "; gotest(test7)
	
	return 0;
*/

	Zexall *zexall = Zexall::Instance();
	zexall->start(0x100);
	cerr << "Ended..." << endl;

	return 0;
}

