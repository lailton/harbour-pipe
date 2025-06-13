/**
 * @file client.prg
 * @author Lailton (lailton@paysoft.com.br)
 * @brief Connect to PIPE
 */

#include "hbpipe.ch"

function main()

	local pPtr := Pipe_Connect( "\\.\pipe\Harbour" )
	local nNum, aMessage
	local lMT := hb_mtvm()
	local pThread, cBuffer
	local nError

	if hb_isPointer( pPtr )

		if lMT
			pThread := hb_threadStart( @Pipe_Message(), pPtr )
		endif

		aMessage := {}
		aadd( aMessage, "Hello from Client side" )
		aadd( aMessage, "Are you there?" )
		aadd( aMessage, "It is a test" )
		aadd( aMessage, replicate( "X", 5 * 1024 ) + "it is an extra testing..." )
		aadd( aMessage, "Closing" )
		aadd( aMessage, "Good Bye" )

		for nNum := 1 to len( aMessage )
			if Pipe_Write( pPtr, aMessage[ nNum ] )
				Pipe_Flush( pPtr )
				if !lMT
					if Pipe_Read( pPtr, @cBuffer )
						? "cBuffer: ", cBuffer
					else
						nError := Pipe_Error( @pPtr )
						if nError != ERROR_IO_PENDING
							? "nError: ", nError
						endif
					endif
				endif
			endif
		next

		Pipe_Flush( pPtr )

		if lMT

			? "5s to finish thread read"

			hb_idleSleep( 5 )
			hb_threadQuitRequest( pThread )
			hb_threadWaitForAll()

		endif

		Pipe_Free( pPtr )

	else

		nError := Pipe_Error()
		if nError == ERROR_PIPE_BUSY
			? "Pipe Busy - Probably already in use"
			? "Use server-mt for multiple access"
		else
			? "Error: ", Pipe_Error()
		endif

	endif

return nil

function Pipe_Message( pPtr )

	local cBuffer
	local nError

	do while .t.

		cBuffer := nil

		if Pipe_Read( pPtr, @cBuffer )
			if hb_isString( cBuffer )
				? "cBuffer: ", cBuffer
			endif
		else
			nError := Pipe_Error()
			if nError != ERROR_IO_PENDING
				? "nError: ", nError
			endif
		endif

		hb_idleSleep( 0.5 )

	enddo

return nil
