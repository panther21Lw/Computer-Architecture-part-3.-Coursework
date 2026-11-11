/*
 * interrupt.c
 *
 *  Created on: Nov 8, 2026
 *      Author: ADMIN
 */

#include <stdint.h>
#include <string.h>

#include "interrupt.h"
#include "drivers/usart.h"

#define Q_LEN					(10U)					// Length of each message queue
#define LAST_RECEIVER_INDEX 	(COMMAND_LEN_MAX - 1U)	// just last possible index in command buffer


/**
 * This function is used to get an address of the next message for transmission.
 *
 * Function returns status of both message queues: normal and prioritized.
 * Note: param. (char *address) is an OUTPUT parameter.
 */
MESSAGE_Q_STATUS_t next_message(const char *);


//uint32_t parity_error = TRANSMISSION_OK;
//uint32_t overrun_error = TRANSMISSION_OK;
//uint32_t framing_error = TRANSMISSION_OK;
//uint32_t noise_error = TRANSMISSION_OK;

//uint32_t transmission_complete = TRANSMISSION_FINISHED;

/**
 * This enumerate defines possible states of state machine for USART-TRANSMITTER.
 * This states are:
 * -- IDLE		The transmitter has finished transmitting the last message from the queue.
 * -- TRANSMIT	The transmitter is transmitting message from the queue now.
 * -- CHECK		The transmitter checks queues to see if there are still messages waiting to be sent.
 * -- WAIT_TC	The last message was sent. Need to check if the transmission is over at the hardware level (TC flag).
 */
typedef enum {
	TRANSMITTER_IDLE		= 0x00UL,		/* Transmitter at IDLE state		*/
	TRANSMITTER_TRANSMIT	= 0x01UL,		/* Transmitter at TRANSMIT state	*/
	TRANSMITTER_CHECK		= 0x02UL,		/* Transmitter at CHECK_Qs state	*/
	TRANSMITTER_WAIT_TC		= 0x03UL		/* Transmitter at WAIT_TC state		*/
} TRANSMITTER_STATE_t;

/**
 * This three variables is the variables for USART-TRANSMITTER state machine.
 */
static uint32_t transmitter_state = TRANSMITTER_IDLE;	// Current state machine state.
static const char *transmitter_message_start;					// Address of the start of the message for transmission.
static uint32_t transmitter_index = 0UL;				// Index of the next char of the message to be sent.


/**
 * This enumerate defines possible states of state machine for USART-RECEIVER.
 * This states are:
 * -- COMMAND_RECEIVING		The receiver is receiving the command character by character.
 * -- COMMAND_READY			The char-by-char received command is packed into a command buffer and ready to be processed.
 * -- ERROR_WARNING			The receiver detected errors during reception and warn user about them.
 * -- WAIT_FOR_ACK			The receiver waits for acknowledge from the user: PRESS ENTER BUTTON -> "\n" symbol.
 */
typedef enum {
	RECEIVER_COMMAND_RECEIVING	= 0x00UL,	/* Receiver at COMMAND_RECEIVING state	*/
	RECEIVER_COMMAND_READY		= 0x01UL,	/* Receiver at COMMAND_READY	 state	*/
	RECEIVER_ERROR_WARNIGN		= 0x02UL,	/* Receiver at ERROR_WARNING	 state	*/
	RECEIVER_WAIT_FOR_ACK		= 0x03UL	/* Receiver at WAIT_FOR_ACK		 state	*/
} RECEIVER_STATE_t;

uint32_t receiver_command_buffer_index;
uint32_t ready_buffer_num;
uint32_t command_buffer_overflow_flag;	/* buffer overflow flag: 1 overflow occurred, 0 not occurred */
/* if the command buffer is full and '\n' not found */


/* Emulating circular buffer or just two-buffer implementation	*/
static char command_buffer1[COMMAND_LEN_MAX];	// buffer for received command
static char command_buffer2[COMMAND_LEN_MAX];	// buffer for received command

static struct command_buffers buffers = {
		.command_buffer1 = command_buffer1,
		.command_buffer2 = command_buffer2,
		.buffer_1_status = BUFFER_NOT_READY,
		.buffer_2_status = BUFFER_NOT_READY
};

/* Active buffer Receiver currently filling	*/


// This function returns pointers to command buffers and their statuses
const struct command_buffers* get_buffers(void) {
	// return the pointer to the structure
	return &buffers;
}

/**
 * This structure defines FIFO circular buffer
 * to implement message queue.
 * Each queue can store only 10 messages.
 */
static struct message_q {
	const char *messages[Q_LEN];	// pointer to start of each message
	uint32_t head;		// index to get the message from the queue
	uint32_t tail;		// index to push the message to the queue
	uint32_t count;		// number of messages in the queue
} normal_q = { .head = 0U, .tail = 0U, .count = 0U },
  prior_q = { .head = 0U, .tail = 0U, .count = 0U };

// This function adds the message to the corresponding queue
// and starts the transmission if not started already
MESSAGE_Q_STATUS_t send_message(const char* message, MESSAGE_PRIOR_t priority)
{
	struct message_q* message_qp;	// pointer to target message queue

	// Select message queue
	if (priority == MESSAGE_INFO) {
		message_qp = &normal_q;
	} else if (priority  == MESSAGE_ERROR) {
		message_qp = &prior_q;
	} else {
		return MESSAGE_WRONG_Q;
	}

	if (message_qp->count < Q_LEN) {	// If there is still room in the queue for a message
		message_qp->messages[message_qp->tail] = message;	// Add the message to corresponding queue
		message_qp->tail++;		// Increment the index for insertion

		// Index ringing
		if (message_qp->tail == Q_LEN) {
			message_qp->tail = 0U;
		}

		message_qp->count++;	// Increment number of messages in the corresponding queue
	} else {
		return MESSAGE_QUEUE_OVERFLOW;	// There is no place in the queue for the message
	}

	// Starting the transmission if not started already.
	if (transmitter_state == TRANSMITTER_IDLE) {
		(void)next_message(transmitter_message_start);	// ignoring the return because any queue can not be empty.
		transmitter_index = 0U;	// reseting the index for transmitter.
		transmitter_state = TRANSMITTER_TRANSMIT;	// select current state of USART-TRANSMITTER state machine

		USART2_TXE_Interrupt_Enable();	// starting the transmission of a next message at the next TXE-interrupt
	}

	return MESSAGE_ADD_SUCCESS;
}

/**
 * This function is used to get an address of the next message for transmission.
 *
 * Function returns status of both message queues: normal and prioritized.
 * Note: param. (char *address) is an OUTPUT parameter.
 */
MESSAGE_Q_STATUS_t next_message(const char *address) {

	struct message_q *message_qp = &prior_q;	// first check the prioritized queue.

	if (message_qp->count == 0U) {	// if prioritized queue is empty
		message_qp = &normal_q;	// switching to not prioritized queue.

		if (message_qp->count == 0U) {	// if normal queue is also empty
			return MESSAGE_QUEUES_EMPTY;	// informing that queues are empty.
		}
	}

	// returning the address of a next message
	address = message_qp->messages[message_qp->head];

	message_qp->head++;	// increment of the index of a next message that will be returned

	// index ringing
	if (message_qp->head == Q_LEN) {
		message_qp->head = 0U;
	}

	message_qp->count--;	// updating the counter of messages

	return MESSAGE_RETURN_SUCCESS;
}


void USART2_IRQHandler(void) {
	uint32_t usart_status = USART2->SR;		// data contains STATUS register

	// TRANSMIT NODE
	// If there is interrupt generated by TXE flag
	if ((transmitter_state == TRANSMITTER_TRANSMIT) &&
		(usart_status & (1UL << 7U))) {
		char symbol = *(transmitter_message_start + transmitter_index);	// obtain next symbol in the message

		if (symbol == '\0') {	// if an end of the message is achieved
			transmitter_state = TRANSMITTER_CHECK;	// changing state to check queues for other messages
			transmitter_index = 0U;	// resetting the index
		} else {
			// Transmit obtained symbol
			USART2->DR = (uint32_t)((uint8_t) symbol);	// writing symbol to TDR
			transmitter_index++;	// increment index so next symbol can be obtained with the same start of message
		}
	}

	// CHECK NODE
	if (transmitter_state == TRANSMITTER_CHECK) {
		// obtaining the state of messages queues and address of the message to send
		MESSAGE_Q_STATUS_t status = next_message(transmitter_message_start);

		if (status == MESSAGE_RETURN_SUCCESS) {
			// If a message for transmitting is available
			// Back to transmission this message
			transmitter_state = TRANSMITTER_TRANSMIT;
		} else 	if (status == MESSAGE_QUEUES_EMPTY) {
			// If a message for transmitting not available
			USART2_TXE_Interrupt_Disable();	// stop generating interrupts by TXE flag
			transmitter_state = TRANSMITTER_WAIT_TC;
			USART2_TC_Interrupt_Enable(); // enable interrupt by TC flag
		}
	}

	// WAIT FOR TC NODE
	if (transmitter_state == TRANSMITTER_WAIT_TC &&
		(usart_status & (1UL << 6U))) {
		USART2_TC_Interrupt_Disable();	// disable interrupt by TC flag
		transmitter_state = TRANSMITTER_IDLE;	// setting the state of TRANSMITTER to IDLE
	}


	uint32_t usart_data	  = USART2->DR;		// data contains DATA	register

}

