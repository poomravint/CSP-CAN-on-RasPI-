#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <csp/csp.h>
#include <csp/drivers/can_socketcan.h>

#define SERVER_ADDR 1  //* Server Address
#define SERVER_PORT 10 //* Server Port

bool done = false;

// TODO : gcc csp_simple_can_rev.c -lcsp -lpthread -o cspsimplecanrev

void *do_route(void *arg) //* Thread fucntion
{
    while (!done)
    {
        csp_route_work(); //* Route packet from the incoming router queue
    }
}

int main(int argc, char *argv[])
{
    csp_iface_t *iface;      //* For set interface
    csp_socket_t sock = {0}; //* Create socket with no specific socket options, e.g. accepts CRC32, HMAC, etc. if enabled during compilation
    pthread_t router_thread; //* For use Pthread
    int ret;

    csp_init(); //* Start CSP

    int bitrate = 500000;
    ret = csp_can_socketcan_open_and_add_interface("can0", CSP_IF_CAN_DEFAULT_NAME, SERVER_ADDR, bitrate, false, &iface);
    //* can0 = device – CAN device name (Linux device).
    //* CSP_IF_CAN_DEFAULT_NAME = ifname – CSP interface name, use #CSP_IF_CAN_DEFAULT_NAME for default name.
    //* SERVER_ADDR = node_id – CSP address of the interface.
    //* bitrate = bitrate – if different from 0, it will be attempted to change the bitrate on the CAN device - this may require increased OS privileges.
    //* false = promisc – if true, receive all CAN frames. If false a filter is set on the CAN device, using device->addr
    //* &iface = return_iface – the added interface.

    //! If error to start CAN will exit this code !//
    if (ret != CSP_ERR_NONE)
    {
        printf("open failed\n");
        exit(1);
    }
    iface->addr = SERVER_ADDR; //* Set Server's address
    iface->is_default = 1;

    csp_conn_print_table(); //* Print connection table
    csp_iflist_print();

    ret = pthread_create(&router_thread, NULL, do_route, NULL); //* Start route thread
    if (ret != 0)
    {
        printf("pthread failed\n");
        exit(1);
    }

    csp_bind(&sock, CSP_ANY);       //* Bind socket to all ports, e.g. all incoming connections will be handled here
    csp_listen(&sock, SERVER_PORT); //* Create a backlog of 10 connections, i.e. up to 10 new connections can be queued

    while (!done) //* While loop for waiting client to connect
    {
        csp_packet_t *req; //* Create packet for store data to send from client
        csp_conn_t *conn;  //* For store connection structure

        conn = csp_accept(&sock, CSP_MAX_TIMEOUT); //! Accept/Waiting new connection
        req = csp_read(conn, CSP_MAX_TIMEOUT);     //* Read packet from connection

        switch (csp_conn_dport(conn)) //* Check destination port of connection
        {
        case 10: //* Destination match with server port

            printf("Packet received on SERVER_PORT: %d\n", *(int *)req->data); //* Show received data from client

            break;

        default: //* Destination doesn't match with server port
            printf("wrong port\n");
            exit(1);
            break;
        }

        csp_buffer_free(req); //* Clear data in req
        csp_close(conn);      //* Close an open connection
    }

    return 0;
}