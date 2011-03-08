/*
 * ARP.c
 *
 *  Created on: 2011-03-06
 *      Author: holman
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#include "ARP.h"
#include "Ethernet.h"
#include "sr_protocol.h"
#include "Defs.h"
#include "sr_router.h"

/*prints the arp header for debugging purposes.*/
//static void printArpPacketHdr(struct sr_arphdr* arphdr);

/*Updates the arp table with the ip mac pair passed in
 * @return 1 if the entry exists in the arp table thus update succeeds
 * 	return 0 if no entry with matching ip exists in the arp table
 */
static int updateArpEntry(struct ip_eth_arp_tbl_entry* arp_tbl, const uint32_t ip, uint8_t* mac);

/*Find the arp table entry whose ip field matches the ip passed in
 * if one exists
 * @return the arp table whose ip field matches the ip passed in
 * 	return NULL if no such entry if no such entry exists
 */
static struct ip_eth_arp_tbl_entry* findArpEntry(struct ip_eth_arp_tbl_entry* arp_tbl, const uint32_t ip);

/*Add and entry into the arp table based on the ip and mac passed in*/
static void addArpEntry(struct sr_if* iface, const uint32_t ip, uint8_t* mac);

/*Reusing the eth frame for the arp request to convert it into
 * the arp response.
 */
static void setupArpResponse(struct sr_arphdr* arphdr, struct sr_if* iface);

void handleArpPacket(struct sr_instance* sr, uint8_t * ethPacket, struct sr_if* iface){

	struct sr_arphdr* arphdr = (struct sr_arphdr*)(ethPacket + sizeof(struct sr_ethernet_hdr));

	assert(arphdr);

	//printArpPacketHdr(arphdr);

	if(ntohs(arphdr->ar_hrd) != ARPHDR_ETHER){
		//hardware address space is not of type ethernet
		//nothing to do with it here
		return;
	}

	if(arphdr->ar_hln != ETHER_ADDR_LEN){
		//hardware address is not 6 bytes long
		//nothing to do with it here
		return;
	}

	if(ntohs(arphdr->ar_pro) != ETHERTYPE_IP){
		//upper layer protocol is not ip
		//nothing to do with it here
		return;
	}

	if(arphdr->ar_pln != IP_ADDR_LEN){
		//ip addr is not 4 bytes long
		//nothing to do with it here
		return;
	}

	int updated_arp_entry = updateArpEntry(iface->ip_eth_arp_tbl, arphdr->ar_sip, arphdr->ar_sha);

	if(arphdr->ar_tip != iface->ip){
		//this arp packet is not targeted for the ip bounded to the interface
		//nothing more to do with the arp packet
		return;
	}

	// now that the arp packet is targeted for the ip bound to the interface
	// that received the packet we first add an arp entry for the source
	// interface if the arp table didn't get updated with the src ip and mac
	// because there isn't an entry in the arp table that has the ip field
	// matching the ip addr of the src interface.
	if(!updated_arp_entry){
		addArpEntry(iface, arphdr->ar_sip, arphdr->ar_sha);
	}

	if(ntohs(arphdr->ar_op) == ARP_REQUEST){
		setupArpResponse(arphdr, iface);
		send_arp_response(sr, arphdr->ar_tha, ethPacket, iface);
	}

}

static void setupArpResponse(struct sr_arphdr* arphdr, struct sr_if* iface){

	arphdr->ar_tip = arphdr->ar_sip;
	MACcpy(arphdr->ar_tha, arphdr->ar_sha);

	arphdr->ar_sip = iface->ip;
	MACcpy(arphdr->ar_sha, iface->addr);

	arphdr->ar_op = htons(ARP_REPLY);
}

static void addArpEntry(struct sr_if* iface, const uint32_t ip, uint8_t* mac){

	struct ip_eth_arp_tbl_entry* arp_entry = (struct ip_eth_arp_tbl_entry*) malloc(sizeof(struct ip_eth_arp_tbl_entry));

	MACcpy(arp_entry->addr, mac);
	time(&(arp_entry->last_modified));

	//append the new entry to the front of the linked list
	//representing the arp table
	arp_entry->next = iface->ip_eth_arp_tbl;
	iface->ip_eth_arp_tbl = arp_entry;
}

static int updateArpEntry(struct ip_eth_arp_tbl_entry* arp_tbl, const uint32_t ip, uint8_t* mac){
	struct ip_eth_arp_tbl_entry* arp_entry = findArpEntry(arp_tbl, ip);
	if(arp_entry == NULL){
		return FALSE;
	}
	else{
		MACcpy(arp_entry->addr, mac);
		time(&(arp_entry->last_modified));
		return TRUE;
	}
}

static struct ip_eth_arp_tbl_entry* findArpEntry(struct ip_eth_arp_tbl_entry* arp_tbl, const uint32_t ip){
	while(arp_tbl){
		if(arp_tbl->ip == ip){
			return arp_tbl;
		}
		arp_tbl = arp_tbl->next;
	}
	//can't find an entry with matching ip
	return NULL;
}

/*static void printArpPacketHdr(struct sr_arphdr* arphdr){
	printf("\n");
	printf("ARP header:\n");
	printf("Hrd addr space: %d\n", ntohs(arphdr->ar_hrd));
	printf("Proto addr space: %d\n", ntohs(arphdr->ar_pro));
	printf("Hrd addr length: %d\n", arphdr->ar_hln);
	printf("Proto addr length: %d\n", arphdr->ar_pln);
	printf("arp op: %d\n", ntohs(arphdr->ar_op));

	printf("Src hrd addr: ");
	printEthAddr(arphdr->ar_sha);
	printf("\n");

	char dotted_ip[INET_ADDRSTRLEN]; //should contain dotted-decimal format of interface ip
	inet_ntop(AF_INET, &(arphdr->ar_sip), dotted_ip, INET_ADDRSTRLEN);
	dotted_ip[INET_ADDRSTRLEN] = '\0';
	printf("Src ip: %s\n", dotted_ip);

	printf("Target hrd addr: ");
	printEthAddr(arphdr->ar_tha);
	printf("\n");

	inet_ntop(AF_INET, &(arphdr->ar_tip), dotted_ip, INET_ADDRSTRLEN);
	dotted_ip[INET_ADDRSTRLEN] = '\0';
	printf("Target ip: %s\n", dotted_ip);
	printf("\n");
}*/
