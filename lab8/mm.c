#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

team_t team = {
    /* Team name */
    "1190501614",
    /* First member's full name */
    "cgh",
    ""
};
/* Basic constants and macros */
#define ALIGN(size) ((((size) + (DSIZE - 1)) / (DSIZE))*DSIZE) 
#define WSIZE 4
#define DSIZE 8
#define INITCHUNKSIZE (1 << 12)
#define CHUNKSIZE (1 << 12)
#define MAX_LEN 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define SET_PTR(p, bp) (*(unsigned int *)(p) = (unsigned int)(bp))

#define PRED_PTR(bp) ((char *)(bp))
#define SUCC_PTR(bp) ((char *)(bp) + WSIZE)

#define PRED(bp) (*(char **)(bp))
#define SUCC(bp) (*(char **)(SUCC_PTR(bp)))

/* Global variables */
static char *heap_listp; /* pointer to first block */
void *List_free[MAX_LEN]; 

static void *extend_heap(size_t size);
static void *coalesce(void *bp);
static void *place(void *bp, size_t size);
static void printblock(void *bp);
static void checkblock(void *bp);
static void AddNode(void *bp, size_t size); 
static void DeleteNode(void *bp); 

int mm_init(void)
{
    int i;
    for (i = 0; i < MAX_LEN; i++) List_free[i] = NULL;
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == NULL) return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); 
    PUT(heap_listp + (DSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (DSIZE+WSIZE), PACK(0, 1)); 
    /* Extend the empty heap with a free block of INITCHUNKSIZE bytes */
    if (extend_heap(INITCHUNKSIZE) == NULL) return -1;
    return 0;
}

/*mm_free - Free a block*/
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    AddNode(bp, size);
    coalesce(bp);
}


static void *extend_heap(size_t size)
{
    char *bp;
    //Allocate an even number of words to maintain alignment
    size = ALIGN(size);
    if ((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;
    //Initialize free block header/footer and the epilogue header
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    AddNode(bp, size);
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    if(prev_alloc)
    {
        return (void*)bp;
    }
    else
    {
       DeleteNode(bp);
       DeleteNode(PREV_BLKP(bp));
       size += GET_SIZE(HDRP(PREV_BLKP(bp)));
       PUT(FTRP(bp), PACK(size, 0));
       PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
       bp = PREV_BLKP(bp);
       AddNode(bp,size);
       return (void*)bp;
    }
}
    

static void AddNode(void *bp, size_t size)
{
    int i = 0;
    void *find_bp = NULL;
    void *insert_bp = NULL;
    if(size > 1<<11)
    {
        size>>=11;
        i=11;
    }

    while ((i < MAX_LEN - 1) && (size > 1)) 
    {
        size >>= 1;
        i=i+1;
    }
    find_bp = List_free[i];
    while ((find_bp != NULL) && (size > GET_SIZE(HDRP(find_bp))))
    {
        insert_bp = find_bp;
        find_bp = PRED(find_bp);
    }
    if (find_bp != NULL)
    {
        if (insert_bp != NULL)
        {
            SET_PTR(PRED_PTR(bp), find_bp);
            SET_PTR(SUCC_PTR(find_bp), bp);
            SET_PTR(SUCC_PTR(bp), insert_bp);
            SET_PTR(PRED_PTR(insert_bp), bp);
        }
        else
        {
            SET_PTR(PRED_PTR(bp), find_bp);
            SET_PTR(SUCC_PTR(find_bp), bp);
            SET_PTR(SUCC_PTR(bp), NULL);
            List_free[i] = bp;
        }
    }
    else
    {
        if (insert_bp != NULL)
        { 
            SET_PTR(PRED_PTR(bp), NULL);
            SET_PTR(SUCC_PTR(bp), insert_bp);
            SET_PTR(PRED_PTR(insert_bp), bp);
        }
        else
        { 
            SET_PTR(PRED_PTR(bp), NULL);
            SET_PTR(SUCC_PTR(bp), NULL);
            List_free[i] = bp;
        }
    }
}


static void DeleteNode(void *bp)
{
    int i = 0;
    size_t size = GET_SIZE(HDRP(bp));
    
    if(size > 1<<11)
    {
        size>>=11;
        i=11;
    }
    while ((i < MAX_LEN - 1) && (size > 1))
    {
        size >>= 1;
        i++;
    }
    
    if (PRED(bp) != NULL)
    {
       
        if (SUCC(bp) != NULL)
        {
            SET_PTR(SUCC_PTR(PRED(bp)), SUCC(bp));
            SET_PTR(PRED_PTR(SUCC(bp)), PRED(bp));
        }
        
        else
        {
            SET_PTR(SUCC_PTR(PRED(bp)), NULL);
            List_free[i] = PRED(bp);
        }
    }
    else
    {
        
        if (SUCC(bp) != NULL) SET_PTR(PRED_PTR(SUCC(bp)), NULL);
        else List_free[i] = NULL;
    }
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc) return bp;//case1
    else if (prev_alloc && !next_alloc) //case2
    {
        DeleteNode(bp);
        DeleteNode(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc) //case3
    {
        DeleteNode(bp);
        DeleteNode(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else //case4
    {
        DeleteNode(bp);
        DeleteNode(PREV_BLKP(bp));
        DeleteNode(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    AddNode(bp, size);
    return bp;
}

void *mm_malloc(size_t size)
{
    char *bp = NULL;
    int i = 0;
    size_t asize;
    if (size == 0) return NULL;
    if (size <= DSIZE) size = 2 * DSIZE; 
    else size = ALIGN(size + DSIZE); 
    asize=size;
    if(asize> 1<<11)
    {
       asize>>=11;
       i=11;
    }
    while (i < MAX_LEN)
    {
        if (((asize <= 1) && (List_free[i] != NULL)))
        {
            bp = List_free[i];
           
            while ((bp != NULL) && ((size > GET_SIZE(HDRP(bp)))))
                bp = PRED(bp);

            if (bp != NULL)
                break;
        }
        asize >>= 1;
        i++;
    }
    
    if (bp == NULL) if ((bp = extend_heap(MAX(size, CHUNKSIZE))) == NULL) return NULL;
    bp = place(bp, size);
    return bp;
}


static void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    size_t diff = csize - asize; 
    DeleteNode(bp);
    if (diff < DSIZE * 2)
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    else if (asize >= 96)
    {
        PUT(HDRP(bp), PACK(diff, 0));
        PUT(FTRP(bp), PACK(diff, 0));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(asize, 1));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 1));
        AddNode(bp, diff);
        return NEXT_BLKP(bp);
    }

    else
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(diff, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(diff, 0));
        AddNode(NEXT_BLKP(bp), diff);
    }
    return bp;
}

void *mm_realloc(void *bp, size_t size)
{
    void *new_p = bp;
    int diff;
    /*Ingore spurious requests*/
    if (size == 0) return NULL;
    if (size <= DSIZE) size = 2 * DSIZE; else size = ALIGN(size + DSIZE); 
    if ((diff = GET_SIZE(HDRP(bp)) - size) >= 0) return bp;
    
    else if (!GET_ALLOC(HDRP(NEXT_BLKP(bp))) || !GET_SIZE(HDRP(NEXT_BLKP(bp))))
    {
        
        if ((diff = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(NEXT_BLKP(bp))) - size) < 0)
        {
            if (extend_heap(MAX(-diff, CHUNKSIZE)) == NULL) return NULL;
            diff += MAX(-diff, CHUNKSIZE);
        }
        
        DeleteNode(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size + diff, 1));
        PUT(FTRP(bp), PACK(size + diff, 1));
    }
    else
    {
        new_p = mm_malloc(size);
        memcpy(new_p, bp, GET_SIZE(HDRP(bp)));
        mm_free(bp);
    }
    return new_p;
}

void mm_checkheap(int verbose)
{
    char *bp = heap_listp;

    if (verbose) printf("Heap (%p):\n", heap_listp);
    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp))) printf("Bad prologue header\n");
    checkblock(heap_listp);
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (verbose)printblock(bp);
        checkblock(bp);
    }
    if (verbose) printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) printf("Bad epilogue header\n");
}

static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));
    if (hsize == 0)
    {
        printf("%p: EOL\n", bp);
        return;
    }
    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp, hsize, (halloc ? 'a' : 'f'), fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp)
{
    if ((size_t)bp % 8) printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp))) printf("Error: header does not match footer\n");
}
