# ezi_str

A Safe, Overflow-Proof String Library, written in C, especially useful in Tiny Microprocessors, but equally useful for PCs.

Example 1:
// declare the barc ezi_str

	EZI_STR  (barc, 20, "");
  
// do a read here from the barcode reader into barc[], getting up to first 20 chars into 'barc'

	some_function_to_read_barcode(barc);

Example 2:
//ezi_calloc(), using the return address

	EZI_STR_T *p1; //declare a pointer
	...
	p1 = ezi_calloc(2000);
