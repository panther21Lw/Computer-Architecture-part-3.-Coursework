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
MESSAGE_Q_STATUS_t next_message(const char **);

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
 * These three variables is the variables for USART-TRANSMITTER state machine.
 */
static uint32_t transmitter_state = TRANSMITTER_IDLE;	// Current state machine state.
static const char *transmitter_message_start;			// Address of the start of the message for transmission.
static uint32_t transmitter_index = 0UL;				// Index of the next char of the message to be sent.



/* Emulating circular buffer or just two-buffer implementation	*/
static char command_buffer1[COMMAND_LEN_MAX];	// buffer for received command
static char command_buffer2[COMMAND_LEN_MAX];	// buffer for received command

static struct command_buffers buffers = {
		.command_buffer1 = command_buffer1,
		.command_buffer2 = command_buffer2,
		.buffer_1_status = BUFFER_NOT_READY,
		.buffer_2_status = BUFFER_NOT_READY
};

// This function returns pointers to command buffers and their statuses
const struct command_buffers* get_buffers(void) {
	// return the pointer to the structure
	return &buffers;
}

// This function sets corresponding buffer state as not ready.
void set_buffer_not_ready(BUFFER_NUM_t buf_num) {
	if (buf_num == BUFFER_1) {
		buffers.buffer_1_status = BUFFER_NOT_READY;
	} else if (buf_num == BUFFER_2) {
		buffers.buffer_2_status = BUFFER_NOT_READY;
	}
}

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
	RECEIVER_ERROR_WARNING		= 0x02UL,	/* Receiver at ERROR_WARNING	 state	*/
	RECEIVER_WAIT_FOR_ACK		= 0x03UL	/* Receiver at WAIT_FOR_ACK		 state	*/
} RECEIVER_STATE_t;

// Variables for USART-RECEIVER state machine.
static RECEIVER_STATE_t receiver_state = RECEIVER_COMMAND_RECEIVING;	// Current state machine state
static uint32_t receiver_command_buffer_index = 0UL;			// Index in command buffer to place character of next command
static char *receiver_active_command_buffer = command_buffer1;
static uint32_t receiver_active_command_buffer_num = 1UL;		// Number of the buffer currently being filled.

/**
 * Noise detected flag.
 * This flag is set (==1) if during reception of any character of a command noise was detected (NF).
 * Possible values are:
 * -- 1 noise was detected;
 * -- 0 noise not detected;
 */
static uint32_t command_noise_flag	= 0UL;

/**
 * Command too long flag.
 * This flag is set (==1) if the receiver did not detect the '\n' symbol at the maximum possible position.
 * Possible values are:
 * -- 1 command too long;
 * -- 0 normal command, flag is not set;
 */
static uint32_t command_too_long_flag = 0UL;

/**
 * Critical error flag.
 * This flag is set (==1) if critical error (PE, ORE, FE) occurred during command reception.
 * Possible values are:
 * -- 1 error occurred;
 * -- 0 error not occurred;
 */
static uint32_t command_crit_err_flag = 0UL;

/* Message to inform about error during reception of the command			*/
const char error_init_mes[] = "ERROR detected during command reception.\n";

/* Message to inform about critical error during reception of the command 	*/
const char critical_error_mes[] = "Critical error detected. Check the wiring and data configuration.\n";

/* Message in case of command too long error	*/
const char command_too_long_mes[] = "Command too long!\n";

/* Message to obtain confirmation from the user	*/
const char error_confirm_mes[] = "Press Enter and re-enter the command.\n";


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
		(void)next_message(&transmitter_message_start);	// ignoring the return because any queue can not be empty.
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
MESSAGE_Q_STATUS_t next_message(const char **address) {

	struct message_q *message_qp = &prior_q;	// first check the prioritized queue.

	if (message_qp->count == 0U) {	// if prioritized queue is empty
		message_qp = &normal_q;	// switching to not prioritized queue.

		if (message_qp->count == 0U) {	// if normal queue is also empty
			return MESSAGE_QUEUES_EMPTY;	// informing that queues are empty.
		}
	}

	// returning the address of a next message
	*address = message_qp->messages[message_qp->head];

	message_qp->head++;	// increment of the index of a next message that will be returned

	// index ringing
	if (message_qp->head == Q_LEN) {
		message_qp->head = 0U;
	}

	message_qp->count--;	// updating the counter of messages

	return MESSAGE_RETURN_SUCCESS;
}


void USART2_IRQHandler(void) {
	uint32_t usart_status = USART2->SR;		// data that contains STATUS register

	// BEGIN OF TRANSMITTER STATE MACHINE
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
		MESSAGE_Q_STATUS_t status = next_message(&transmitter_message_start);

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
	// END OF TRANSMITTER STATE MACHINE

	// BEGIN OF RECEIVER STATE MACHINE
	if (usart_status & (1UL << 5U)) {		// if interrupt occurred by RXNE event

		uint32_t usart_data	= USART2->DR;		// data that contains DATA register
		char received_symbol = (uint8_t)(usart_data &= 0xFFUL);	// select only least significant byte

		// Receiver state machine handler
		// COMMAND_RECEIVING NODE
		if (receiver_state == RECEIVER_COMMAND_RECEIVING) {
			// If there is no critical error (PE, ORE, FE) during character reception
			if (!(usart_status & 0x0BUL)) {
				/* command too long error detection	*/
				if ((receiver_command_buffer_index == LAST_RECEIVER_INDEX) &&
					received_symbol != '\n') {	// if '\n' (command terminator) to detected at maximum expected place
					receiver_state = RECEIVER_ERROR_WARNING;	// change the state to handle the error
					command_too_long_flag = 1UL;				// setting the command too long flag
				} else if (received_symbol == '\n') { /* command terminator ('\n') detection	*/
					receiver_active_command_buffer[receiver_command_buffer_index] = '\0';	// inserting string terminator
					receiver_state = RECEIVER_COMMAND_READY;
				} else {
					// add the next byte of command to command buffer
					receiver_active_command_buffer[receiver_command_buffer_index] = received_symbol;
					receiver_command_buffer_index++;	// increment index

					/* noise during transmission detection	*/
					if (usart_status & 0x04UL) {
						// if NF flag is set
						command_noise_flag = 1UL;	// setting the flag for the command currently being received
					}
				}
			} else {
				// If critical errors (PE, ORE, FE) detected during character reception
				receiver_state = RECEIVER_ERROR_WARNING;	// changing state of receiver state machine to warn the user
				command_crit_err_flag = 1UL;	// set the critical error flag.
			}
		}

		// ERROR WARNING NODE
		if (receiver_state == RECEIVER_ERROR_WARNING) {
			receiver_command_buffer_index = 0UL;		// resetting command buffer index: received command is invalid.
			send_message(error_init_mes, MESSAGE_ERROR);	// Informing about error

			if (command_too_long_flag) {
				send_message(command_too_long_mes, MESSAGE_ERROR);	// Informing about too long command
				command_too_long_flag = 0UL;	// resetting command too long flag.
			}

			if (command_crit_err_flag) {
				send_message(critical_error_mes, MESSAGE_ERROR);
				command_crit_err_flag = 0UL;	// resetting critical error flag.
			}

			send_message(error_confirm_mes, MESSAGE_ERROR);
			command_noise_flag = 0UL;			// resetting noise detected flag.

			receiver_state = RECEIVER_WAIT_FOR_ACK;		// change the state to wait for confirmation from the user.
		}

		// WAIT FOR ACKNOWLEDGE NODE
		if (receiver_state == RECEIVER_WAIT_FOR_ACK) {
			// if confirmation is received
			if (received_symbol == '\n') {
				receiver_state = RECEIVER_COMMAND_RECEIVING;	// back to receiving new command.
			}
		}

		// COMMAND READY NODE
		if (receiver_state == RECEIVER_COMMAND_READY) {
			if (receiver_active_command_buffer_num == 1UL) {
				/* if buffer #1 was selected for filling	*/
				// changing status of buffer #1 as ready (considering the noise)
				command_noise_flag ? (buffers.buffer_1_status = BUFFER_READY_NOISE) : (buffers.buffer_1_status = BUFFER_READY);
				receiver_active_command_buffer = command_buffer2;	// changing buffer for filling to buffer #2
				receiver_active_command_buffer_num = 2UL;			// changing number of active buffer to #2
				buffers.buffer_2_status = BUFFER_NOT_READY;			// set status for the buffer #2 as NOT READY

			} else if (receiver_active_command_buffer_num == 2UL) {
				/* if buffer #2 was selected to filling		*/
				// changing status of buffer #2 as ready (considering the noise)
				command_noise_flag ? (buffers.buffer_2_status = BUFFER_READY_NOISE) : (buffers.buffer_2_status = BUFFER_READY);
				receiver_active_command_buffer = command_buffer1;	// changing buffer for filling to buffer #1
				receiver_active_command_buffer_num = 1UL;			// changing number of active buffer to #1
				buffers.buffer_1_status = BUFFER_NOT_READY;			// set status for the buffer #1 as NOT READY
			}
			receiver_command_buffer_index = 0UL;	// resetting index to receive new command from begin
			command_noise_flag = 0UL;	// resetting command noise flag for next command

			receiver_state = RECEIVER_COMMAND_RECEIVING;	// back to receiving new command.
		}
	}
	// END OF RECEIVER STATE MACHINE
}

