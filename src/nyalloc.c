#include "nyalloc.h"
#include <stdio.h>
#include <unistd.h>

typedef struct Block {
  struct Block *next;
  struct Block *prev;
  size_t size;
  unsigned char status;
} Block;

static Block *mewmory;

static inline size_t align(size_t size) {
  return (size + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);
}

static Block *first_fit(size_t size) {
  Block *cur = mewmory;
  while (cur && (!cur->status || cur->size < size))
    cur = cur->next;
  return cur;
}

static Block *new_block(size_t size) {
  size_t total_size = align(size + sizeof(Block));
  void *mem = sbrk(total_size);
  if (mem == (void *)-1)
    return NULL;

  Block *block = (Block *)mem;
  block->size = total_size - sizeof(Block);
  block->status = 0;
  block->next = NULL;
  block->prev = NULL;

  if (mewmory) {
    block->next = mewmory;
    mewmory->prev = block;
  }
  mewmory = block;

  return block;
}

static void split_block(Block *block, size_t size) {
  size = align(size);
  block->status = 0;
  if (block->size <= size + sizeof(Block) + sizeof(size_t))
    return;

  size_t fst_size = sizeof(Block) + size;
  Block *second = (Block *)((char *)block + fst_size);

  second->size = block->size - fst_size;
  second->status = 1;
  second->next = block->next;
  second->prev = block;

  block->size = size;
  block->next = second;
}

static void join_blocks(Block *block) {
  if (block->prev) block = block->prev;
  for (int i = 0; i < 2; ++i)
    if (block->next && block->next->status) {
      block->size += sizeof(Block) + block->next->size;
      block->next = block->next->next;
    }
}

void *nyalloc(size_t size) {
  if (size == 0)
    return NULL;
  Block *block = first_fit(size);

  if (block)
    split_block(block, size);
  else {
    block = new_block(size);
    if (!block)
      return NULL;
  }

  return (void *)((char *)block + sizeof(Block));
}

void fwee(void *p) {
  if (!p)
    return;

  Block *block = (Block *)((char *)p - sizeof(Block));
  block->status = 1;

  join_blocks(block);
}
