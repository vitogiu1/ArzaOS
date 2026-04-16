#ifndef PORTS_H
#define PORTS_H

void port_byte_out(unsigned short port, unsigned char data);
unsigned char port_byte_in(unsigned short port);

#endif