/*
Jacob Branchaud
Fall 2016
Created for CS620 Operating Systems at The University of New Hampshire - Durham

The purpose of this assignment was to simulate the behavior of an LRU (Least Recently Used) Cache.
 It will initially allocate N frames to simulate main memory (entered into standard input). The 
 program will then take "pages" from a file and will attempt to put them in a free allocated frame. 
 If no frames are available, the caching algorithm will take effect. The first step is to evict a 
 page that has been used least recently. The program knows this based on the location in the LRU 
 linked list. It finds a spot for the data by hashing the page number of the page to be stored. 
 When accessing the bucket associated with the hash (doubly-linked list), If there is nothing in 
 the bucket yet, the new page becomes the head of the list. If there is something there, it will 
 become the new head of the list, shifting everything down by 1 page. At the end of the program's 
 execution, The number of page faults will print to stdout, as well as the contents of the simulated 
 main memory.
*/

#include <stdlib.h>
#include <stdio.h>
#include "lru.h"

int frames_in_mm;
int pages_in_vm;
int num_faults;

int input_workload [200];

// initialization function
// creates a single linked list of FRAME structures
void init_it() {
	int i, j;
	FRAME *pCurrent, *pNew;
	for (i = 0; i < frames_in_mm; i++) {
		// allocate memory for a structure
		if ((pNew = (FRAME *) malloc( sizeof(FRAME) )) == NULL) {
			perror( "Error allocating memory" );
			exit( 1 );
		}
	//** Next -- initialize the fields in frame;
	//** set frame number 0 to M-1
		//printf("%d\n", i); //debugging
		pNew->frame_num = i;

		//set the head of the free list
		if( i == 0 ){
			fm_free_list = pNew;
			pCurrent = pNew;
		} 
		//set the next element of the free list to the one created
		else{
			pCurrent->f_free_next = pNew;
			pCurrent = pNew;
		}
	}
}

FRAME* get_frame_free( ){
	if( fm_free_list == NULL ) return NULL;
	else{
		FRAME* head = fm_free_list;
		fm_free_list = fm_free_list->f_free_next;
		return head;
	}
}

void lru_remove( ){
	//printf("%ld\n", fm_lru_tail->page_num );
	//printHash();
	FRAME* lru = fm_lru_tail;
	int page = lru->page_num;
	int hashbucket = hash(page);

	//remove from the lru list
	fm_lru_tail = fm_lru_tail->f_lru_prev;
	fm_lru_tail->f_lru_next = NULL;
	

	if(!lru->f_hash_prev) page_table[hashbucket] = page_table[hashbucket]->f_hash_next;
	if(lru->f_hash_prev) lru->f_hash_prev->f_hash_next = lru->f_hash_next;
	if(lru->f_hash_next) lru->f_hash_next->f_hash_prev = lru->f_hash_prev;	

	fm_free_list = lru;

	fm_free_list->f_hash_prev = NULL;
	fm_free_list->f_hash_next = NULL;
	fm_free_list->f_lru_prev = NULL;
	fm_free_list->f_lru_next = NULL;
	fm_free_list->f_free_next = NULL;
}

void printHash(){
	//printf("%s\n", "hash: ");
	for(int i = 0; i < ihash_size; i++){
		FRAME* cur = page_table[i];
		while(cur != NULL){
			printf("%ld ", cur->page_num);
			cur = cur->f_hash_next;
		}
		printf("\n");
	}
	printf("\n");
}

void printLRU(){
	FRAME* cur = fm_lru_head;
	while(cur){
		printf("%ld ", cur->page_num );
		cur = cur->f_lru_next;
	}
	printf("\n");
}

//function given to us
// int check_page_table(long page_num, FRAME **p_p_frame_pkt){

// }

//my own function without passing table in
int check_page_table(long page_num){
	//printHash();
	int hashbucket = hash(page_num);
	FRAME* cur = page_table[hashbucket];
	while(cur != NULL){
		if(cur->page_num == page_num){
			if(cur == fm_lru_head) return 1;
			else if(cur == fm_lru_tail){
				fm_lru_tail = fm_lru_tail->f_lru_prev;
				fm_lru_tail->f_lru_next = NULL;
			}
			if(cur->f_lru_prev) cur->f_lru_prev->f_lru_next = cur->f_lru_next;
			if(cur->f_lru_next) cur->f_lru_next->f_lru_prev = cur->f_lru_prev;
			cur->f_lru_next = fm_lru_head;
			fm_lru_head->f_lru_prev = cur;
			fm_lru_head = cur;
			return 1;
		}
		cur = cur->f_hash_next;
	}


	FRAME* freeFrame = get_frame_free( );
	if(freeFrame != NULL){ //still free memory frames
		put_pageframe_in_memory(page_num, freeFrame);
	}
	else{
		//remove something from lru
		lru_remove();
		put_pageframe_in_memory(page_num, get_frame_free());
		//printLRU(); //debugging
	}
	printFrames();
	num_faults++;
}

void put_pageframe_in_memory(long page_num, FRAME *p_frame_pkt){
	//printf("%s%d\n", "hash ", hash(page_num) );
	int hashbucket = hash(page_num);
	p_frame_pkt->page_num = page_num;

	if( page_table[hashbucket] == NULL ){ //bucket is empty		
		page_table[hashbucket] = p_frame_pkt;
	}
	else{
		page_table[hashbucket]->f_hash_prev = p_frame_pkt;
		p_frame_pkt->f_hash_next = page_table[hashbucket];
		page_table[hashbucket] = p_frame_pkt;
	}

	if(fm_lru_head == NULL){
		fm_lru_head = fm_lru_tail = p_frame_pkt;
	}
	else{
		fm_lru_head->f_lru_prev = p_frame_pkt;
		p_frame_pkt->f_lru_next = fm_lru_head;
		fm_lru_head = p_frame_pkt;
	}
}


int hash(long pageNumber){
	long iHashKey;
	iHashKey = pageNumber + 1;
	return(iHashKey % ihash_size);
}

void printFrames(){
	for( int i = 0; i < frames_in_mm; i++){
		FRAME* cur = fm_lru_head;
		while(cur != NULL){
			if(cur->frame_num == i) printf("%ld ", cur->page_num);
			cur = cur->f_lru_next;
		}
	}
	printf("\n");
}

int main( int argc, char const *argv[]){
	//read input into variables
	scanf("%d", &ihash_size);
	scanf("%d", &frames_in_mm);
	scanf("%d", &pages_in_vm);

	init_it();

	int page;
	//int index = 0;
	while(scanf("%d", &page) != EOF){ //as we scan in following inputs
		check_page_table(page);
	}

	printf("%d\n", num_faults);
	FRAME* cur = fm_lru_head;
	while( cur != NULL ){
		printf("%ld ", cur->page_num );
		cur = cur->f_lru_next;
	}
	printf("\n");
	
}