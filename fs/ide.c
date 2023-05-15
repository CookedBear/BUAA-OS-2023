/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

u_int ssd_bitmap[32];
u_int ssd_cleanmap[32];
u_int ssd_logic[32];
u_int ssd_physics[32];

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in
// 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET',
//  'DEV_DISK_OPERATION_READ', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS',
//  'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
  u_int begin = secno * BY2SECT;
  u_int end = begin + nsecs * BY2SECT;

  // sample: ide_read(0, blockno * SECT2BLK, va, SECT2BLK);

  for (u_int off = 0; begin + off < end; off += BY2SECT) {
    uint32_t temp = diskno;
    /* Exercise 5.3: Your code here. (1/2) */
    panic_on(-E_INVAL == syscall_write_dev(&temp,
                                           (DEV_DISK_ADDRESS | DEV_DISK_ID),
                                           sizeof(temp))); // select disk_id
    temp = begin + off;
    panic_on(-E_INVAL ==
             syscall_write_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_OFFSET),
                               sizeof(temp))); // select reading offset_address
    temp = DEV_DISK_OPERATION_READ;
    panic_on(-E_INVAL ==
             syscall_write_dev(&temp,
                               (DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION),
                               sizeof(temp))); // select operation

    panic_on(-E_INVAL == syscall_read_dev(&temp,
                                          (DEV_DISK_ADDRESS | DEV_DISK_STATUS),
                                          sizeof(temp))); // get reading result
    if (temp == 0) {
      panic_on(temp == 0);
    } else {
      panic_on(-E_INVAL ==
               syscall_read_dev((void *)((u_int)dst + off),
                                (DEV_DISK_ADDRESS | DEV_DISK_BUFFER),
                                DEV_DISK_BUFFER_LEN)); // pull from buffer
    }
  }
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in
// 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
  u_int begin = secno * BY2SECT;
  u_int end = begin + nsecs * BY2SECT;

  for (u_int off = 0; begin + off < end; off += BY2SECT) {
    uint32_t temp = diskno;
    /* Exercise 5.3: Your code here. (2/2) */
    panic_on(-E_INVAL == syscall_write_dev(src + off,
                                           (DEV_DISK_ADDRESS | DEV_DISK_BUFFER),
                                           BY2SECT));
    panic_on(-E_INVAL == syscall_write_dev(&temp,
                                           (DEV_DISK_ADDRESS | DEV_DISK_ID),
                                           sizeof(temp)));
    temp = begin + off;
    panic_on(-E_INVAL == syscall_write_dev(&temp,
                                           (DEV_DISK_ADDRESS | DEV_DISK_OFFSET),
                                           sizeof(temp)));
    temp = DEV_DISK_OPERATION_WRITE;
    panic_on(-E_INVAL ==
             syscall_write_dev(&temp,
                               (DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION),
                               sizeof(temp)));

    panic_on(-E_INVAL == syscall_read_dev(&temp,
                                          (DEV_DISK_ADDRESS | DEV_DISK_STATUS),
                                          sizeof(temp)));
    if (temp == 0) {
      panic_on(temp == 0);
    }
  }
}

u_int get_physics(u_int logic_no) {
  for (int ssdno = 0; ssdno < 32; ssdno++) {
    if (ssd_logic[ssdno] == logic_no) {
      return ssd_physics[ssdno];
    }
  }
  return 0xffffffff;
}

void erase(u_int physics_no) {
  char buf[512];
  ssd_cleanmap[physics_no]++;
  ssd_bitmap[physics_no] = 1;
  for (int i = 0; i < 512; i++) {
    buf[i] = 0;
  }
  ide_write(0, physics_no, buf, 1);
}

u_int alloc_physics() {
  u_int target_no = 1000;
  u_int erase_time = 1000;
  u_int exchange_no = 1000;

  char buf[512];

  for (int ssdno = 0; ssdno < 32; ssdno++) {
    if (ssd_cleanmap[ssdno] < erase_time && ssd_bitmap[ssdno] == 1) {
      target_no = ssdno;
      erase_time = ssd_cleanmap[ssdno];
    }
  }

  if (erase_time < 5) {
    return target_no;
  }

  erase_time = 1000;
  for (int ssdno = 0; ssdno < 32; ssdno++) {
    if (ssd_bitmap[ssdno] == 0 && ssd_cleanmap[ssdno] < erase_time) {
      exchange_no = ssdno;
      erase_time = ssd_cleanmap[ssdno];
    }
  }

  for (int i = 0; i < 512; i++) {
    buf[i] = 0;
  }

  ssd_cleanmap[target_no]++;
  ide_read(0, target_no, buf, 1);
  ide_write(0, exchange_no, buf, 1);

  ssd_bitmap[target_no] = 0;
  for (int ssdno = 0; ssdno < 32; ssdno++) {
    if (ssd_physics[ssdno] == exchange_no) {
      ssd_physics[ssdno] = target_no;
    }
  }

  return exchange_no;
}

void ssd_init() {
  for (int ssdno = 0; ssdno < 32; ssdno++) {
    ssd_bitmap[ssdno] = 1;
    ssd_physics[ssdno] = (u_int)0xffffffff;
    ssd_logic[ssdno] = (u_int)0xffffffff;
  }
}

int ssd_read(u_int logic_no, void *dst) {
  u_int physics_no = get_physics(logic_no);
  if (physics_no == (u_int)0xffffffff) {
    return -1;
  }

  ide_read(0, physics_no, dst, 1);

  return 0;
}

void ssd_write(u_int logic_no, void *src) {
  u_int physics_no = (u_int)0xffffffff;
  for (int ssdno = 0; ssdno < 32; ssdno++) {
    if (ssd_logic[ssdno] == logic_no) {
      physics_no = ssd_physics[ssdno];
      if (physics_no != (u_int)0xffffffff) {
        ssd_erase(logic_no);
        ssd_logic[ssdno] = (u_int)0xffffffff;
        ssd_logic[ssdno] = (u_int)0xffffffff;
      }
    }
  }

  u_int save_no = alloc_physics();
  ssd_bitmap[save_no] = 0;
  // debugf("alloc physics %d\n", save_no);
  for (int ssdno = 0; ssdno < 32; ssdno++) {
    if (ssd_logic[ssdno] == (u_int)0xffffffff) {
      ssd_logic[ssdno] = logic_no;
      ssd_physics[ssdno] = save_no;
      break;
    }
  }

  ide_write(0, save_no, src, 1);
}

void ssd_erase(u_int logic_no) {
  u_int physics_no = get_physics(logic_no);
  if (physics_no == (u_int)0xffffffff) {
    return;
  }

  for (int ssdno = 0; ssdno < 32; ssdno++) {
    if (ssd_logic[ssdno] == logic_no) {
      erase(ssd_physics[ssdno]);
      ssd_logic[ssdno] = (u_int)0xffffffff;
      ssd_physics[ssdno] = (u_int)0xffffffff;
    }
  }
  return;
}
