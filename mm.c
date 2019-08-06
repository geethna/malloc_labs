#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "bi0s",
    /* First member's full name */
    "Geethna T K",
    /* First member's email address */
    "geethna.teekey@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define IN_USE 1
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(block_meta)))
#define is_free(size) (size&0x00000000)
#define BLOCK_MEM(ptr) ((void *)((unsigned long)ptr + sizeof(block_meta)))
#define get_chunk(p) (p - (void *)SIZE_T_SIZE)

typedef struct block_meta {
  size_t size;
  struct block_meta *next;
  struct block_meta *prev;
}block_meta;

int start_size = 0x1000;
void *free_base = NULL;
struct block_meta *current;
struct block_meta *free_list;

void *request_space(size_t size){
    void *p = mem_sbrk(ALIGN(size));
    if (p == (void *)-1)
        return NULL;
    return p;
}
void *del(block_meta *ptr){
    if(!ptr->prev){         //if its head
        if(ptr->next){      //if its not the only chunk in the list
            free_base = ptr->next;
        }
        else{               //if its the only chunk in the list
            free_base = NULL;
        }
    }
    else{                   //if its the last chunk
        ptr->prev->next = ptr->next;
    }
    if(ptr->next){          //if its in the middle
        ptr->next->prev = ptr->prev;
    }
    return NULL;
}

void *add(block_meta *ptr){
    ptr->next = NULL;
    ptr->prev = NULL;
    return NULL;
}
void *split(block_meta *block,size_t size){
    void *mem_block = BLOCK_MEM(block);
    block_meta *newptr = (block_meta *) ((unsigned long)mem_block + size);
    newptr->size = block->size - (size + sizeof(block_meta));
    block->size = size;
    return newptr;
}
void *find_space(size_t size){
    size = ALIGN(size);
    block_meta *block;
    block = free_base;
    while(block){
        if(block->size>=size+SIZE_T_SIZE){
            del(block);
            if(block->size == size)
                return block;
            void *newptr = split(block,size);
            add(newptr);        //adding the split chunk to the free_list
            return block;     //returning the chunk we got
        }
        else{           //if we did not get the relatable size chunk
          block = block->next;
        }
    }
    return NULL;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void *block = NULL;
    if(size<=0)
        return NULL;
    block = find_space(size);
    if(!block){
        block = request_space(size+SIZE_T_SIZE);
        if(!block)
            return NULL;
      }
    current = (block_meta *)block;
    current->size = ALIGN(size+IN_USE);
    current->next = NULL;
    current->prev = NULL;
    return (void *)((char *)current+SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */

void mm_free(void *ptr)
{
    ptr = (void *)get_chunk(ptr);
    if(!free_base){
        free_base = ptr;
        free_list = free_base;
    }
    else{
        free_list->next = ptr;
        free_list = free_list->next;
    }
    free_list->size = free_list->size - IN_USE;
    free_list->next = NULL;
    merge(free_base);
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
