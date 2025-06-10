#ifndef _HB_PIPE_CH_
	#define _HB_PIPE_CH_

	#define ERROR_PIPE_CONNECTED               535 // Client is already connected.
	#define ERROR_PIPE_BUSY                    231 // All pipe instances are busy.
	#define ERROR_PIPE_LISTENING               536 // Pipe is waiting for a connection (rarely returned).
	#define ERROR_PIPE_NOT_CONNECTED           233 // Pipe is not connected.
	#define ERROR_BROKEN_PIPE                  109 // The client has disconnected; the pipe is broken.
	#define ERROR_NO_DATA                      232 // The pipe is being closed and no more data is available.
	#define ERROR_BAD_PIPE                     230 // Pipe state is invalid.
	#define ERROR_PIPE_DISCONNECTED            240 // Pipe is disconnected.
	#define ERROR_PIPE_CLOSING                 241 // Pipe is being closed.
	#define ERROR_PIPE_EMPTY                   246 // Read attempted on empty pipe with non-blocking mode.
	#define ERROR_MORE_DATA                    234 // More data is available (e.g., message exceeds buffer).

#endif
