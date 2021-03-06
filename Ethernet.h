/*
 * Ethernet.h
 *
 *  Created on: 2011-03-06
 *      Author: holman
 */
#include "sr_router.h"

#define BROAD_CAST_MAC 255


/*Handle the eth packet received.*/
void handleEthFrame(struct sr_instance* sr,
        uint8_t * ethPacket,
        unsigned int len,
        char* interface);

/*Prints the mac address to stdout in hex with
 * format xx-xx-xx-xx-xx-xx
 */
void printEthAddr(uint8_t* eth_hdr);

/*Check the mac address passed to see if it is a broadcast address
 * @return 1 if it is broad cast address, return 0 otherwise
 *
 */
int isBroadCastMAC(const uint8_t* macAddr);

/*Compare two mac address to see if they are the same.
 * @return: 1 if they are the same, return 0 otherwise.
 */
int MACcmp(const uint8_t* macAddr1, const uint8_t* macAddr2);

/*Copy a mac address form src to dest*/
void MACcpy(uint8_t* dest, uint8_t* src);

/*Set a broadcast mac address to the mac addr buffer passed in.*/
void setBroadCastMAC(uint8_t* mac_buff);

/*Send an eth frame encapsulating a arp message
 * @param sr the simple router instance
 * @param dest_mac address of the interface where this
 * 		arp message is to go
 * @param eth_frame the eth frame encapsulating the arp
 * 		message
 * @param iface the interface on the router where the arp
 * 		message is to be sent
 * @param payload_len the size in bytes of the arp message
 * 		(not including the size of the eth frame header)
 * */
void sendEthFrameContainingArpMsg(struct sr_instance* sr, uint8_t* dest_mac, uint8_t * eth_frame, struct sr_if* iface, unsigned int payload_len);

/*Send an eth frame encapsulating an arp request
 * @param sr the router instance
 * @param arp_request the arp request to be sent
 * @param iface the interface on this router where the eth frame
 * 		is sent from
 * @param len the size of the arp request in bytes
 */
void ethSendArpRequest(struct sr_instance* sr, uint8_t * arp_request, struct sr_if* iface, unsigned int payload_len);

/*Send an IP datagram
 * @param sr the router instance
 * @param dest_mac the mac addr of the target interface where
 * 		the data is to be sent to
 * @param ip_datagram the datagram to be sent
 * @param iface the interface where the datagram
 * 		is to be sent out from
 * @param len the size of the datagram in bytes
 */
void ethSendIPDatagram(struct sr_instance* sr, uint8_t* dest_mac, uint8_t * ip_datagram, struct sr_if* iface, unsigned int len);

/*Sends an eth frame containing an ip datagram in
 * its data field
 * @param sr the router instance
 * @param dest_mac the mac address of the interface
 * 		where the eth frame is to be sent to
 * @param iface the interface on this router that is
 * 		used to send out the eth frame
 * @param payload_len the size of the ip datagram in bytes
 */
void sendEthFrameContainingIPDatagram(struct sr_instance* sr, uint8_t* dest_mac, uint8_t * eth_frame, struct sr_if* iface, unsigned int payload_len);
