# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define PAGE_NUM 256
# define PAGE_SIZE 256
# define TLB_SIZE 16
# define FRAME_SIZE 256
# define FRAME_NUM 128

//Empty Frame List
typedef struct empty_frame_node {
	int frame_id;
	struct empty_frame_node *next;
}empty_frame_node;

empty_frame_node *head = NULL;
empty_frame_node *tail = NULL;

char memory[FRAME_NUM * FRAME_SIZE];
int frame_LRU[FRAME_NUM];		
char buf[FRAME_SIZE];
FILE *fp_bs;					//for backing store

int TLB_page[TLB_SIZE];
int TLB_frame[TLB_SIZE];
int TLB_LRU[TLB_SIZE];
int TLB_hit_cnt;				//TLB hit count

int page_table[PAGE_NUM];
int page_table_vi[PAGE_NUM];	//valid-invalid
int page_fault_cnt;				//page fault count

void init_empty_frame_list();
int get_empty_frame();
void clean_empty_frame_list();

void init_memory();
int add_page_into_memory(int page_id);
char access_memory(int frame_id, int offset);
void clean_memory();
void update_frame_LRU(int frame_id);

void init_TLB();
int get_TLB_frame_id(int page_id);
void add_TLB_item(int page_num, int frame_num);
void update_TLB_LRU(int dex);
void delete_TLB_item(int page_id, int frame_id);

void init_page_table();
int get_frame_id(int page_id);
void invalid_page_table_item(int frame_id);

//Initialize the empty frame list
void init_empty_frame_list() {
	for (int i = 0; i < FRAME_NUM; ++ i) //Add each empty frame
	{
		if (head == NULL && tail == NULL)//no empty frame
		{
			tail = (empty_frame_node *) malloc (sizeof(empty_frame_node));
			tail -> frame_id = i;
			tail -> next = NULL;
			head = tail;
		} 
		else//add to tail
		{
			tail->next = (empty_frame_node *)malloc(sizeof(empty_frame_node));
			tail->next->frame_id = i;
			tail->next->next = NULL;
			tail = tail->next;
		}
	}
}

//Get an empty frame from the empty frame list
int get_empty_frame() {
	int frame_id;
	//no empty frame
	if (head == NULL && tail == NULL) return -1;
	//one empty frame
	if (head == tail) {
		frame_id = head -> frame_id;
		free(head);
		head = tail = NULL;
		return frame_id;
	}
	//more than one
	empty_frame_node *p=head;	
	frame_id = head -> frame_id;
	head = head -> next;
	free(p);
	return frame_id;
}

// Clean the empty frame list.
void clean_empty_frame_list() {
	if (head == NULL && tail == NULL) return;
	struct empty_frame_node *p;
	while (head != tail) 
	{
		p = head;
		head = head -> next;
		free(p);
	}
	free(head);
	head = tail = NULL;
}

//Initialize memory
void init_memory() {
	fp_bs = fopen("BACKING_STORE.bin", "rb");
	if (fp_bs == NULL) 
	{
		fprintf(stderr, "  ERROR: Open backing store file error\n");
		exit(1);
	}
	//Initialize the empty frame list
	init_empty_frame_list();
	//initialize LRU record
	for (int i = 0; i < FRAME_NUM; ++ i)
		frame_LRU[i] = 0;
}

int add_page_into_memory(int page_id) {
	fseek(fp_bs, page_id * FRAME_SIZE, SEEK_SET);
	fread(buf, sizeof(char), FRAME_SIZE, fp_bs);//read data to buffer
	int frame_id = get_empty_frame();		//get an empty frame for the page
	if (frame_id == -1)						//not found LRU replacement
	{		
		for (int i = 0; i < FRAME_NUM; ++ i)
			if (frame_LRU[i] == FRAME_NUM)	//for replacement
			{
				frame_id = i;
				break;
			}
		invalid_page_table_item(frame_id);
	}
	for (int i = 0; i < FRAME_SIZE; ++ i)	//put data into memory
	{
		memory[frame_id * FRAME_SIZE + i] = buf[i];
	}
	//update_frame_LRU(frame_id);
	for (int i = 0; i < FRAME_NUM; ++ i)	//update frame LRU record
	{
		if (frame_LRU[i] > 0) 
			frame_LRU[i]++;
	}
	frame_LRU[frame_id] = 1;				//latest access
	return frame_id;
}

void update_frame_LRU(int frame_id)
{
	for (int i = 0; i < FRAME_NUM; ++ i)	//update frame LRU record
		if (frame_LRU[i] > 0 && frame_LRU[i] < frame_LRU[frame_id])
			++ frame_LRU[i];
	frame_LRU[frame_id] = 1;
}

char access_memory(int frame_id, int offset) 
{
	char rst = memory[frame_id * FRAME_SIZE + offset];
	update_frame_LRU(frame_id);
	return rst;
}

void clean_memory() 
{
	clean_empty_frame_list();
	fclose(fp_bs);
}
//initialize TLB
void init_TLB() 
{
	TLB_hit_cnt = 0;
	for (int i = 0; i < TLB_SIZE; ++ i) 
	{
		TLB_page[i] = 0;
		TLB_frame[i] = 0;
		TLB_LRU[i] = 0;
	}
}

int get_TLB_frame_id(int page_id) 
{
	int dex = -1;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if (TLB_LRU[i] > 0 && TLB_page[i] == page_id) 
		{
			dex = i;
			break;
		}
	if (dex == -1) return -1;	// TLB not hit.
	++ TLB_hit_cnt;				// TLB hit.
	update_TLB_LRU(dex);		//update TLB LRU record
	return TLB_frame[dex];
}

void update_TLB_LRU(int dex)
{
	for (int i = 0; i < TLB_SIZE; ++ i)	//update TLB LRU record
		if (TLB_LRU[i] > 0 && TLB_LRU[i] < TLB_LRU[dex])
			++ TLB_LRU[i];
	TLB_LRU[dex] = 1;
}

// add TLB
void add_TLB_item(int page_id, int frame_id) 
{
	int dex = -1;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if(TLB_LRU[i] == 0) {
			dex = i;
			break;
		}
	if (dex == -1) 		// LRU replacement.
	{
		for (int i = 0; i < TLB_SIZE; ++ i)
			if(TLB_LRU[i] == TLB_SIZE) {
				dex = i;
				break;
			}
	}
	
	TLB_page[dex] = page_id;
	TLB_frame[dex] = frame_id;
	for (int i = 0; i < TLB_SIZE; ++ i)	//update TLB LRU record
		if (TLB_LRU[i] > 0) ++ TLB_LRU[i];
	TLB_LRU[dex] = 1;
}

// Delete TLB
void delete_TLB_item(int page_id, int frame_id) 
{
	int dex = -1;
	for (int i = 0; i < TLB_SIZE; ++ i)
		if(TLB_LRU[i] && TLB_page[i] == page_id && TLB_frame[i] == frame_id) {
			dex = i;
			break;
		}
	if (dex == -1) return;
	for (int i = 0; i < TLB_SIZE; ++ i)	//update TLB LRU record
		if (TLB_LRU[i] > TLB_LRU[dex]) -- TLB_LRU[i];
	TLB_LRU[dex] = 0;//empty
}

//initialize page table
void init_page_table() {
	page_fault_cnt = 0;
	for (int i = 0; i < PAGE_NUM; ++ i) {
		page_table[i] = 0;
		page_table_vi[i] = 0;
	}
}

int get_frame_id(int page_id)
{
	if (page_id < 0 || page_id >= PAGE_NUM) return -1;
	
	//TLB
	int TLB_frame_id = get_TLB_frame_id(page_id);
	if (TLB_frame_id != -1) return TLB_frame_id;
	
	//TLB NOT HIT -> page table
	if (page_table_vi[page_id] == 1) //page table hit
	{
		add_TLB_item(page_id, page_table[page_id]);
		return page_table[page_id];
	} 
	else // Page fault.
	{
		page_fault_cnt++;
		page_table[page_id] = add_page_into_memory(page_id);
		page_table_vi[page_id] = 1;//vailid
		add_TLB_item(page_id, page_table[page_id]);
		return page_table[page_id];
	}
}

void invalid_page_table_item(int frame_id)
{
	int page_id = -1;
	for (int i = 0; i < PAGE_NUM; ++ i)
		if(page_table_vi[i] && page_table[i] == frame_id) {
			page_id = i;
			break;
		}
	if (page_id == -1) {
		fprintf(stderr, "  ERROR: PAGE_ID Error\n");
		exit(1);
	}
	page_table_vi[page_id] = 0;
	delete_TLB_item(page_id, frame_id);
}

int main(int argc, char *argv[]) {	
	if (argc != 2) 
	{
		fprintf(stderr, "  ERROR: Invalid input\n");
		return 1;
	}

	FILE *fp_in = fopen(argv[1], "r");
	if(fp_in == NULL) 
	{
		fprintf(stderr, "  ERROR: File Error\n");
		return 1;
	}
	
	FILE *fp_out = fopen("output.txt", "w");
	if (fp_out == NULL) 
	{
		fprintf(stderr, "  ERROR: File Error\n");
		return 1;
	}

	init_page_table();
	init_TLB();
	init_memory();
		
	int addr, page_id, offset, frame_id, res, cnt = 0;
	while(~fscanf(fp_in, "%d", &addr)) {
		++ cnt;
		addr = addr & 0x0000ffff;
		offset = addr & 0x000000ff;
		page_id = (addr >> 8) & 0x000000ff;
		frame_id = get_frame_id(page_id);
		res = (int) access_memory(frame_id, offset);
		fprintf(fp_out, "Virtual address: %d Physical address: %d Value: %d\n", addr, (frame_id << 8) + offset, res);
	}
	
	fprintf(stdout, "Statistics:\n  TLB hit rate: %.4f %%\n  Page fault rate: %.4f %%\n", 100.0 * TLB_hit_cnt / cnt, 100.0 * page_fault_cnt / cnt);
	
	clean_memory();
	fclose(fp_in);
	fclose(fp_out);
	return 0;
}
/*
make
./vmm addresses.txt
./checker
*/


