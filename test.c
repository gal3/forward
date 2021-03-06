#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "test.h"
#include "sr_router.h"
#include "sr_if.h"
#include "ARP.h"
#include "ip.h"
#include "icmp.h"
#include "check.h"

/*--------------------------------------------------------------------- 
 * Method: testmethod for debug and learning purposes
 *
 *---------------------------------------------------------------------*/
void testmethod(struct sr_instance* sr, uint8_t * packet, unsigned int len, char* interface){
		
		struct sr_ethernet_hdr* e_hdr = 0;	//init
    struct sr_arphdr*       a_hdr = 0;	//init
		struct ip*							ip_hdr = 0; //init

		struct sr_if* iface = sr_get_interface(sr, interface); //packet is from which interface?
		struct sockaddr_in sa;
		sa.sin_addr.s_addr = iface->ip;//assign binary value to s_addr
		char dotted_ip[INET_ADDRSTRLEN]; //should contain dotted-decimal format of interface ip
		inet_ntop(AF_INET, &(sa.sin_addr), dotted_ip, INET_ADDRSTRLEN);
		printf("*** -> Received packet of length %d on interface %s with ip %s\n",len, interface, dotted_ip);
		printf("Interface MAC address: ");
		for(int i=0; i<6; i++){//mac address of interface
			printf("%x ",iface->addr[i]);
		}
		printf("\n");
		e_hdr = (struct sr_ethernet_hdr*)packet;//cast ethernet header
		printf("ethernet type: %x\n",(int)(e_hdr->ether_type)); //ethernet type
		printf("ethernet type: %x\n",(int)ntohs(e_hdr->ether_type)); //ethernet type
		printf("ethernet type: %x\n",(int)(e_hdr->ether_type)); //ethernet type
		printf("Destination mac address: ");
		for(int i=0; i<ETHER_ADDR_LEN; i++){//destination mac address
			printf("%x",e_hdr->ether_dhost[i]);
			printf(" ");
		}
		printf("\n");
		printf("Source mac address: ");
		for(int i=0; i<ETHER_ADDR_LEN; i++){//source mac address
			printf("%x",e_hdr->ether_shost[i]);
			printf(" ");
		}
		printf("\n");

		if((e_hdr->ether_type) == htons(ETHERTYPE_ARP)){
			//it's an arp packet!
			a_hdr = (struct sr_arphdr*)(packet + sizeof(struct sr_ethernet_hdr)); //cast arpheader
			printf("ARP source address: ");
			for(int i=0; i<6; i++){
				printf("%x",a_hdr->ar_sha[i]);
				printf(" ");
			}
			printf("\n");
			sa.sin_addr.s_addr = a_hdr->ar_sip;
			inet_ntop(AF_INET, &(sa.sin_addr), dotted_ip, INET_ADDRSTRLEN);
			printf("Sender IP: %d %s\n",a_hdr->ar_sip, dotted_ip);
		}
		else if ((e_hdr->ether_type) == htons(ETHERTYPE_IP)){
			printf("Examining IP packet\n");
			ip_hdr = (struct ip*)(packet + sizeof(struct sr_ethernet_hdr));
			printf("IP protocol: %x\n",ip_hdr->ip_p);
		}
}

void testSendArpRequest(struct sr_instance* sr){
	struct sr_if* iface = sr_get_interface(sr, "eth1");
	uint32_t ip;
	inet_pton(AF_INET, "171.67.245.29", &ip);
	uint8_t mac[ETHER_ADDR_LEN];
	resolveMAC(sr, ip, iface, mac);
}

void testSendIcmpMsg(struct sr_instance* sr){
	uint8_t icmp_msg_len = ICMP_HDR_LEN;
	uint8_t* icmp_msg = (uint8_t*)malloc(icmp_msg_len);
	assert(icmp_msg);

	struct icmphdr* icmp_msg_hdr = (struct icmphdr*)icmp_msg;

	icmp_msg_hdr->icmp_type = 8;
	icmp_msg_hdr->icmp_code = 0;

	uint32_t* unused = (uint32_t*)(icmp_msg + 4);

	*unused = 0;

	icmp_msg_hdr->icmp_checksum = 0;

	int checksum = csum((uint16_t*) icmp_msg, icmp_msg_len);

	icmp_msg_hdr->icmp_checksum = checksum;


	char* dest_doted_ip = "171.67.245.29";
	uint32_t dest_ip = 0;
	inet_pton(AF_INET, dest_doted_ip, &dest_ip);

	char* src_doted_ip = "171.67.245.28";
	uint32_t src_ip = 0;
	inet_pton(AF_INET, src_doted_ip, &src_ip);

	ipSendIcmpMessageWithSrcIP(sr, icmp_msg, icmp_msg_len, dest_ip, src_ip);

	if(icmp_msg){
		free(icmp_msg);
	}
}
