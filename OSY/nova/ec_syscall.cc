#include "bits.h"
#include "ec.h"
#include "ptab.h"

#define MAX_THREADS 64

typedef enum {
  sys_print = 1,
  sys_sum = 2,
  sys_nbrk = 3,
  sys_thr_create = 4,
  sys_thr_yield = 5,
} Syscall_numbers;

static Ec *thread_buffer[MAX_THREADS];
static int thread_count = 0;
static int current_thread_idx = 0;

/**
 * System call handler
 *
 * @param[in] a Value of the `al` register before the system call
 */
void Ec::syscall_handler(uint8 a) {
  Sys_regs *r = static_cast<Sys_regs *>(current->sys_regs());
  Syscall_numbers number = static_cast<Syscall_numbers>(a);

  if (thread_count == 0) {
    thread_buffer[0] = current;
    thread_count = 1;
    current_thread_idx = 0;
  }

  switch (number) {
  case sys_print: {
    char *data = reinterpret_cast<char *>(r->esi);
    unsigned len = r->edi;
    for (unsigned i = 0; i < len; i++) {
      printf("%c", data[i]);
    }
    break;
  }

  case sys_sum: {
    int first_number = r->esi;
    int second_number = r->edi;
    r->eax = first_number + second_number;
    break;
  }

  case sys_nbrk: {
    mword address = reinterpret_cast<mword>(r->esi);
    mword old_brk = current->break_current;
    mword old_top = align_up(old_brk, PAGE_SIZE);
    mword new_top = align_up(address, PAGE_SIZE);

    // Return current brk - nop
    if (address == 0 || address == old_brk) {
      r->eax = static_cast<unsigned int>(old_brk);
      break;
    }
    // Address is out of range
    if (address > static_cast<mword>(0xBFFFF000) || address < current->break_min) {
      r->eax = 0;
      break;
    }
    // Update brk
    if (new_top > old_top) { // Expand address space
      bool ok = true;

      for (mword va = old_top; va < new_top; va += PAGE_SIZE) {
        void *page = Kalloc::allocator.alloc_page(1, Kalloc::FILL_0);
        if (!page) {
          ok = false;
          break;
        }
        mword phys = Kalloc::virt2phys(page);
        mword attr = Ptab::PRESENT | Ptab::USER | Ptab::RW;
        if (!Ptab::insert_mapping(va, phys, attr)) {
          Kalloc::allocator.free_page(page);
          ok = false;
          break;
        }
      }

      if (!ok) {
        for (mword va = old_top; va < new_top; va += PAGE_SIZE) {
          mword pte = Ptab::get_mapping(va);
          if (!(pte & Ptab::PRESENT)) continue;

          Ptab::insert_mapping(va, 0, 0);
          mword phys = pte & ~PAGE_MASK;
          void *kpage = Kalloc::phys2virt(phys);
          Kalloc::allocator.free_page(kpage);
        }
        r->eax = 0;
      } else {
        current->break_current = address;
        r->eax = static_cast<unsigned int>(old_brk);
      }
    } else { // Shrink address space
      for (mword va = new_top; va < old_top; va += PAGE_SIZE) {
        mword pte = Ptab::get_mapping(va);
        if (!(pte & Ptab::PRESENT)) continue;

        Ptab::insert_mapping(va, 0, 0);
        mword phys = pte & ~PAGE_MASK;
        void *kaddr = Kalloc::phys2virt(phys);
        Kalloc::allocator.free_page(kaddr);
      }

      if (address & (PAGE_SIZE - 1)) {
        mword last_page_start = align_dn(address, PAGE_SIZE);
        mword pte = Ptab::get_mapping(last_page_start);
        if (pte & Ptab::PRESENT) {
          void *kaddr = Kalloc::phys2virt(pte & ~PAGE_MASK);
          for (unsigned i = address & (PAGE_SIZE - 1); i < PAGE_SIZE; i++) {
            reinterpret_cast<unsigned char *>(kaddr)[i] = 0;
          }
        }
      }

      current->break_current = address;
      r->eax = static_cast<unsigned int>(old_brk);
    }

    break;
  }

  case sys_thr_create: {
    if (thread_count >= MAX_THREADS) {
      r->eax = 1;
      break;
    }

    // Create new thread
    Ec *new_thread = new Ec();
    if (!new_thread) {
      r->eax = 1;
      break;
    }

    // Init registers
    Exc_regs *new_regs = static_cast<Exc_regs *>(new_thread->sys_regs());
    *new_regs = *reinterpret_cast<Exc_regs *>(current->sys_regs());
    new_regs->edx = r->esi; // start_routine
    new_regs->ecx = r->edi; // stack_top
    new_regs->eax = 0;

    // Insert into circular buffer
    for (int i = thread_count; i > current_thread_idx + 1; i--) {
      thread_buffer[i] = thread_buffer[i - 1];
    }
    thread_buffer[current_thread_idx + 1] = new_thread;
    thread_count++;

    r->eax = 0;
    break;
  }

  case sys_thr_yield: {
    if (thread_count > 1) {
      thread_buffer[current_thread_idx] = current;
      current_thread_idx = (current_thread_idx + 1) % thread_count;
      thread_buffer[current_thread_idx]->make_current();
    }
    break;
  }

  default: printf("Unknown syscall %d\n", number); break;
  };

  ret_user_sysexit();
}