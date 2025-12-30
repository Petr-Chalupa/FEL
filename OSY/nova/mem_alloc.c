#include "mem_alloc.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static inline void *nbrk(void *address);

#ifdef NOVA

/**********************************/
/* nbrk() implementation for NOVA */
/**********************************/

static inline unsigned syscall2(unsigned w0, unsigned w1) {
  asm volatile("   mov %%esp, %%ecx    ;"
               "   mov $1f, %%edx      ;"
               "   sysenter            ;"
               "1:                     ;"
               : "+a"(w0)
               : "S"(w1)
               : "ecx", "edx", "memory");
  return w0;
}

static void *nbrk(void *address) { return (void *)syscall2(3, (unsigned)address); }
#else

/***********************************/
/* nbrk() implementation for Linux */
/***********************************/

#include <unistd.h>

static void *nbrk(void *address) {
  void *current_brk = sbrk(0);
  if (address != NULL) {
    int ret = brk(address);
    if (ret == -1) return NULL;
  }
  return current_brk;
}

#endif

typedef struct Mem_header {
  unsigned long size;
  bool is_free;
  struct Mem_header *next;
} Mem_header;

Mem_header *head = NULL;
Mem_header *tail = NULL;

void *my_malloc(unsigned long size) {
  if (size == 0) return NULL;

  // Try to find suitable block inside the list
  Mem_header *curr = head;
  while (curr) {
    if (curr->is_free && curr->size >= size) {
      // Split block if large enough
      if (curr->size >= size + sizeof(Mem_header) + 1) {
        Mem_header *new_block = (Mem_header *)((char *)(curr + 1) + size);
        new_block->size = curr->size - size - sizeof(Mem_header);
        new_block->is_free = 1;
        new_block->next = curr->next;
        curr->size = size;
        curr->next = new_block;
        if (tail == curr) {
          tail = new_block;
        }
      }

      curr->is_free = 0;
      return (void *)(curr + 1);
    }
    curr = curr->next;
  }

  // Extend the heap
  unsigned long total_size = sizeof(Mem_header) + size;
  void *current_brk = nbrk(0);
  if (nbrk((char *)current_brk + total_size) == NULL) {
    return NULL;
  }

  Mem_header *header = (Mem_header *)current_brk;
  header->size = size;
  header->is_free = 0;
  header->next = NULL;

  if (!head) {
    head = header;
  }
  if (tail) {
    tail->next = header;
  }
  tail = header;

  return (void *)(header + 1);
}

int my_free(void *address) {
  if (!address) return -1;

  Mem_header *header = ((Mem_header *)address) - 1;
  void *curr_brk = nbrk(0);

  // Check validity of address
  Mem_header *curr = head;
  bool found = false;
  while (curr) {
    if (curr == header) {
      found = true;
      if (curr->is_free) return -1; // Double-free
      break;
    }
    curr = curr->next;
  }
  if (!found) return -1; // Invalid pointer

  header->is_free = 1;

  // Join with next block if free
  if (header->next && header->next->is_free) {
    void *expected_next = (char *)(header + 1) + header->size;
    if ((void *)header->next == expected_next) {
      Mem_header *old_next = header->next;
      header->size += sizeof(Mem_header) + old_next->size;
      header->next = old_next->next;
      if (tail == old_next) {
        tail = header;
      }
    }
  }

  // Join with previous block if free
  Mem_header *prev = NULL;
  curr = head;
  while (curr && curr != header) {
    prev = curr;
    curr = curr->next;
  }
  if (prev && prev->is_free) {
    void *expected_current = (char *)(prev + 1) + prev->size;
    if ((void *)header == expected_current) {
      prev->size += sizeof(Mem_header) + header->size;
      prev->next = header->next;
      if (tail == header) {
        tail = prev;
      }
      header = prev;
    }
  }

  // Release memory if at the end of heap
  if ((char *)(header + 1) + header->size == curr_brk) {
    if (head == tail) {
      head = tail = NULL;
    } else {
      Mem_header *tmp = head;
      while (tmp) {
        if (tmp->next == header) {
          tmp->next = NULL;
          tail = tmp;
          break;
        }
        tmp = tmp->next;
      }
    }
    nbrk((char *)curr_brk - (sizeof(Mem_header) + header->size));
  }

  return 0;
}