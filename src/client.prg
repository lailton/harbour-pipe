/**
 * @file client.prg
 * @author Lailton (lailton@paysoft.com.br)
 * @brief Connect to PIPE
 */

#include "hbpipe.ch"

function main()

	local pPtr := Pipe_Connect( "\\.\pipe\Harbour" )
	local pMessage, nError

	if hb_isPointer( pPtr )

		if hb_mtvm()
			pMessage := hb_threadStart( @PIPE_Message(), pPtr )
		endif

		hb_idleSleep( 0.5 )
		if !Pipe_Write( pPtr, "Hello from Client side" )
			? "Error: ", PIPE_Error()
		endif

		hb_idleSleep( 0.5 )
		if !Pipe_Write( pPtr, "Are you there?" )
			? "Error: ", PIPE_Error()
		endif


		hb_idleSleep( 0.5 )
		if !Pipe_write( pPtr, "It is a test" )
			? "Error: ", PIPE_Error()
		endif


		hb_idleSleep( 0.5 )
		if !Pipe_write( pPtr, replicate( "X", 5 * 1024 ) + "it is an extra testing..." ) // Long
			? "Error: ", PIPE_Error()
		endif


		hb_idleSleep( 0.5 )
		if !Pipe_write( pPtr, "Closing" )
			? "Error: ", PIPE_Error()
		endif


		hb_idleSleep( 0.5 )
		if !Pipe_write( pPtr, "Good Bye" )
			? "Error: ", PIPE_Error()
		endif

		if hb_mtvm()
			hb_idleSleep( 0.5 )
			hb_threadQuitRequest( pMessage )
			hb_threadWaitForAll()
		endif

		hb_idleSleep( 4 )

		Pipe_Free( pPtr )

	else

		nError := PIPE_Error()
		if nError == ERROR_PIPE_BUSY
			? "Pipe Busy - Probably already in use"
			? "Use server-mt for multiple access"
		else
			? "Error: ", PIPE_Error()
		endif

	endif

return nil

function PIPE_Message( pPtr )

	local cBuffer
	local nError

	do while .t.

		cBuffer := nil

		if PIPE_Read( pPtr, @cBuffer )
			if hb_isString( cBuffer )
				? "cBuffer: ", cBuffer
			endif
		else
			nError := PIPE_Error()
			if nError != 187
				? nError
			endif
		endif

		hb_idleSleep( 0.5 )

	enddo

return nil
