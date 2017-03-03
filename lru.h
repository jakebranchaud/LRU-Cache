#ifndef LRU_H
#define LRU_H

#define FRAME struct memory_frame
#define HASH_SIZE 1013 //Maximum hash size; do not input greater

int ihash_size;
// read from standard input
FRAME
{
	FRAME *f_hash_next;
	FRAME *f_hash_prev;
	FRAME *f_free_next;
	FRAME *f_lru_next;
	FRAME *f_lru_prev;
	long page_num;
	long frame_num;
};

FRAME *page_table[HASH_SIZE];
FRAME *fm_free_list;
FRAME *fm_lru_head;
FRAME *fm_lru_tail;

int hash(long pageNumber);
void put_pageframe_in_memory(long page_num, FRAME *p_frame_pkt);
int check_page_table(long page_num);
FRAME* get_frame_free( );
void init_it();
void printHash();
void printFrames();

#endif // LRU_H_