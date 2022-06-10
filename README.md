ezi\_str
========

A Safe, Overflow-Proof String Library, written in C, especially useful in Tiny Microprocessors, but equally useful for PCs.

Example using a passed-in ezi_str, and a locally declared ezi_str:

	void BarcodeReader( byte _channel, byte _state, EZI_STR_T *response )
	{
		EZI_STR( command, 6, "" );
		byte seq_status;

		ezi_snprintf( command, "%c%c", _state ? '1' : '0', hex36range( _channel ) );
		acc_comms( command, response, &seq_status );

		SetAccessoryStatus( _channel, _state, seq_status );
	}
