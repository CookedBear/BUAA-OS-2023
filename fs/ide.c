/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

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
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	//sample: ide_read(0, blockno * SECT2BLK, va, SECT2BLK);

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (1/2) */
		syscall_write_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_ID), sizeof(temp)); //select disk_id
		temp = begin + off;
		syscall_write_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_OFFSET), sizeof(temp)); // select reading offset_address
		temp = DEV_DISK_OPERATION_READ;
		syscall_write_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION), sizeof(temp)); // select operation

		syscall_read_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_STATUS), sizeof(temp)); // get reading result
		if (temp == 0) {
			panic_on(temp == 0);
		} else {
			syscall_read_dev((void *) ((u_int) dst + off), (DEV_DISK_ADDRESS | DEV_DISK_BUFFER), BY2SECT); // pull from buffer
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
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
		syscall_write_dev(src + off, (DEV_DISK_ADDRESS | DEV_DISK_BUFFER), BY2SECT);
		syscall_write_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_ID), sizeof(temp));
		temp = begin + off;
		syscall_write_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_OFFSET), sizeof(temp));
		temp = DEV_DISK_OPERATION_WRITE;
		syscall_write_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_START_OPERATION), sizeof(temp));

		syscall_read_dev(&temp, (DEV_DISK_ADDRESS | DEV_DISK_STATUS), sizeof(temp));
		if (temp == 0) {
			panic_on(temp == 0);
		}
	}
}
