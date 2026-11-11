/*
 * interrupt.h
 *
 *  Created on: Nov 8, 2026
 *      Author: ADMIN
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

/**
 * Maximum command length in characters.
 */
#define COMMAND_LEN_MAX		(20U + 1U)	// 20 for a command characters and 1 for '\0' to mark the end of a string.

//typedef enum {
//	TRANSMISSION_OK			= 0x00UL,		/* Transmission completed successfully	*/
//	TRANSMISSION_ERROR		= 0x01UL,		/* Error occurred during transmission	*/
//	TRANSMISSION_BUSSY		= 0x02UL,		/* Data transfer in progress			*/
//	TRANSMISSION_FINISHED	= 0x03UL		/* Data transfer finished				*/
//} TRANSMISSION_STATUS_t;

typedef enum {
	BUFFER_NOT_READY		= 0x00UL,		/* No buffer is ready							*/
	//BUFFER_1_READY			= 0x01UL,		/* Command in Buffer #1 is ready for processing	*/
	//BUFFER_2_READY			= 0x02UL		/* Command in Buffer #2 is ready for processing	*/
	BUFFER_READY			= 0x01UL,		/* Buffer can be processed by the main state machine */
	/* Buffer can be processed by the main state machine
	 * but noise was detected during reception of corresponding buffer
	 */
	BUFFER_READY_NOISE		= 0x02UL
} BUFFER_STATUS_t;

typedef enum {
	MESSAGE_ADD_SUCCESS		= 0x00UL,		/* Message has been added to the corresponding queue 		*/
	MESSAGE_QUEUE_OVERFLOW 	= 0x01UL,		/* Message was not added to the queue: the queue is full	*/
	MESSAGE_WRONG_Q			= 0x02UL,		/* Mentioned message queue does not exist					*/
	MESSAGE_QUEUES_EMPTY	= 0x03UL,		/* Both message queues are empty							*/
	MESSAGE_RETURN_SUCCESS	= 0x04UL		/* Message was returned successfully						*/
} MESSAGE_Q_STATUS_t;

typedef enum {
	MESSAGE_INFO			= 0x00UL,		/* Message that contains usual information	*/
	MESSAGE_ERROR			= 0x01UL		/* Message that contains info about error	*/
} MESSAGE_PRIOR_t;

/**
 * This structure describes buffers for:
 * -- storing received commands;
 * -- statuses of corresponding buffers;
 */
struct command_buffers{
	char *command_buffer1;			// pointer to buffer #1 contains command
	char *command_buffer2;			// pointer to buffer #2 contains command
	uint32_t buffer_1_status;		// status  of buffer #1
	uint32_t buffer_2_status;		// status  of buffer #2
};

// This function returns pointers to command buffers and their statuses
const struct command_buffers* get_buffers(void);

// This function adds the message to the corresponding queue
MESSAGE_Q_STATUS_t send_message(const char* message, MESSAGE_PRIOR_t priority);

#endif /* INTERRUPT_H_ */
