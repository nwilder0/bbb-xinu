/*
 * select.h
 *
 *  Created on: Sep 23, 2015
 *      Author: nathan
 */

#ifndef INCLUDE_SELECT_H_
#define INCLUDE_SELECT_H_

// FD_ZERO

// FD_ISSET

// fd_set

struct fd_set {
	uint32 access_type;
	uint32 device_mask;
	uint32 is_set;
};

#define FD_ACCESS_TYPE_READ 0
#define FD_ACCESS_TYPE_WRITE 1

#define FD_ISSET(d,fds) \
	(((fds)->device_mask & (1<<d)) && ((fds)->is_set & (1<<d)))

#define FD_ZERO(fds) \
	do {
		(fds)->device_mask = 0;
		(fds)->is_set = 0;
	} while(0)

#define FD_SET(d, fds) \
	(fds)->device_mask = fds->device_mask | (1<<d)


#endif /* INCLUDE_SELECT_H_ */
