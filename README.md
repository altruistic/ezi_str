ezi\_str
========

A Safe, Overflow-Proof String Library, written in C, especially useful
in Tiny Microprocessors, but equally useful for PCs.\
\
Example with `ezi_str` constructs highlighted:

`void BarcodeReader( byte _channel, byte _state, EZI_STR_T *response )`\
`{`\
`    EZI_STR( command, 6, "" );`\
`    byte seq_status;`\
``\
`    ezi_snprintf( command, "%c%c", _state ? '1' : '0', hex36range( _channel ) );`\
`    device_comms( command, response, &seq_status );`\
``\
`    // update the device register`\
`    SetAccessoryOutput( _channel, _state, seq_status );`\
`}`\
