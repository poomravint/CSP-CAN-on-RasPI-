#include <stdio.h>
#include <stdlib.h>
#include <libsocketcan.h>

#include <csp/csp.h>
#include <csp/drivers/can_socketcan.h>

#define SERVER_ADDR 1  //* Server Address
#define SERVER_PORT 10 //* Server Port
#define CLIENT_ADDR 2  //* Address address

bool done = false;

// TODO : gcc csp_simple_can_sen.c -lcsp -o cspsimplecansen

int main(int argc, char *argv[])
{
    csp_iface_t *iface;   //* For set interface
    csp_conn_t *conn;     //* For store connection structure
    csp_packet_t *packet; //* Packet for store data to send to server
    csp_packet_t *reply;  //* Packet for store reply from server

    int ret;          //* For check error
    int data_to_send; //* For store data to send

    csp_init(); //* Start CSP

    int bitrate = 500000; //* Bitrate config
    ret = csp_can_socketcan_open_and_add_interface("can0", CSP_IF_CAN_DEFAULT_NAME, CLIENT_ADDR, bitrate, false, &iface);
    //* can0 = device – CAN device name (Linux device).
    //* CSP_IF_CAN_DEFAULT_NAME = ifname – CSP interface name, use #CSP_IF_CAN_DEFAULT_NAME for default name.
    //* CLIENT_ADDR = node_id – CSP address of the interface.
    //* bitrate = bitrate – if different from 0, it will be attempted to change the bitrate on the CAN device - this may require increased OS privileges.
    //* false = promisc – if true, receive all CAN frames. If false a filter is set on the CAN device, using device->addr
    //* &iface = return_iface – the added interface.

    //! If error to start CAN will exit this code !//
    if (ret != CSP_ERR_NONE)
    {
        csp_print("open failed\n");
        exit(1);
    }

    iface->addr = CLIENT_ADDR; //* Set client's address
    iface->is_default = 1;

    csp_conn_print_table(); //* Print connection table
    csp_iflist_print();

    data_to_send = 20000;       //! Set data to send !//
    packet = csp_buffer_get(0); //* Get free buffer from task context

    *(int *)(packet->data) = data_to_send; //* Store value of data_to_send in packet to send to Server

    packet->length = 4; //* Set data length (Bytes) MAX : 251

    conn = csp_connect(CSP_PRIO_NORM, SERVER_ADDR, SERVER_PORT, 0, CSP_O_NONE); //* Connect to server
    printf("Connect!!!!!!!!\n");
    csp_send(conn, packet); //* Send packet to server

    csp_close(conn); //* Close an open connection

    return 0;
}