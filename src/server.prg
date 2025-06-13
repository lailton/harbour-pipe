/**
 * @file server.prg
 * @author Lailton (lailton@paysoft.com.br)
 * @brief Create PIPE to receive communication between exes, with -mt will accept multiple connection
 */

#include "hbpipe.ch"

function main()

	local pPtr

	do while .t.
		pPtr := PIPE_Create( "\\.\pipe\Harbour" )
		if hb_isPointer( pPtr )
			if PIPE_Wait( pPtr )
				if hb_mtvm()
					hb_threadStart( @PIPE_Handler(), pPtr )
				else
					PIPE_Handler( pPtr )
				endif
			else
				PIPE_Free( pPtr )
			endif
		endif
	enddo

return nil

function PIPE_Handler( pPtr )

	local cBuffer, lResult, nError
	local nReceive := 0, cSent

	do while .t.

		cBuffer := nil
		lResult := PIPE_Read( pPtr, @cBuffer )

		if !lResult
			nError := PIPE_Error()
			if nError == ERROR_NO_DATA .or. nError == ERROR_BROKEN_PIPE
				exit
			endif
		endif

		if hb_isString( cBuffer )
			? "cBuffer: ", cBuffer
			cSent := "Received! " + time() + " " + alltrim( str( ++nReceive ) )
			if PIPE_Write( pPtr, cSent )
				PIPE_Flush( pPtr )
				? "sent: ", cSent
			else
				nError := PIPE_Error()
				if nError == ERROR_NO_DATA .or. nError == ERROR_BROKEN_PIPE
					exit
				else
					? "Error write: ", nError
				endif
			endif
		endif

	enddo

	PIPE_Disconnect( pPtr )

return nil
