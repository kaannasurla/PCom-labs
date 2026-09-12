#include <arpa/inet.h> /* ntoh, hton and inet_ functions */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib.h"
#include "protocols.h"
#include <string.h>

/* Routing table */
struct route_table_entry *rtable;
int rtable_len;

/* Mac table */
struct mac_entry *mac_table;
int mac_table_len;

/*
 Returns a pointer (eg. &rtable[i]) to the best matching route, or NULL if there
 is no matching route.
*/
struct route_table_entry *get_best_route(uint32_t ip_dest) {
	/* TODO 2.2: Implement the LPM algorithm */
	/* We can iterate through rtable for (int i = 0; i < rtable_len; i++). Entries in
	/* the rtable are in network order already */
	
	for (int i = 0; i < rtable_len; i++) {

		/* Cum tabela este sortată, primul match este prefixul ce mai specific */
		if (rtable[i].prefix == (ip_dest & rtable[i].mask)) {
			return &rtable[i];
		}
	}
	return NULL;
}

//todo 2.4
in

struct mac_entry *get_mac_entry(uint32_t given_ip) {
	/* TODO 2.4: Iteratincludee through the MAC table and search for an entry
	 * that matches given_ip. */

	/* We can iterate thrpigh the mac_table for (int i = 0; i <
	 * mac_table_len; i++) */
	for (int i = 0; i < mac_table_len; i++) {

		/* Cum tabela este sortată, primul match este prefixul ce mai specific */
		if (mac_table[i].ip == given_ip) {
			return &mac_table[i];
		}
	}
	
	return NULL;
}

int main(int argc, char *argv[])
{
	int interface;
	char packet[MAX_LEN];
	int packet_len;

	/* Don't touch this */
	init();

	/* Code to allocate the MAC and route tables */
	rtable = malloc(sizeof(struct route_table_entry) * 100);
	/* DIE is a macro for sanity checks */
	DIE(rtable == NULL, "memory");

	mac_table = malloc(sizeof(struct  mac_entry) * 100);
	DIE(mac_table == NULL, "memory");
	
	/* Read the static routing table and the MAC table */
	rtable_len = read_rtable("rtable.txt", rtable);
	mac_table_len = read_mac_table(mac_table);

	while (1) {
		/* We call get_packet to receive a packet. get_packet returns
		the interface it has received the data from. And writes to
		len the size of the packet. */
		interface = recv_from_all_links(packet, &packet_len);
		DIE(interface < 0, "get_message");
		printf("We have received a packet\n");
		
		/* Extract the Ethernet header from the packet. Since protocols are
		 * stacked, the first header is the ethernet header, the next header is
		 * at m.payload + sizeof(struct ether_header) */
		struct ether_header *eth_hdr = (struct ether_header *) packet;
		struct iphdr *ip_hdr = (struct iphdr *)(packet + sizeof(struct ether_header));

		/* Check if we got an IPv4 packet */
		if (eth_hdr->ether_type != ntohs(ETHERTYPE_IP)) {
			printf("Ignored non-IPv4 packet\n");
			continue;
		}

		/* TODO 2.1: Check the ip_hdr integrity using ip_checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)) */
		int ok = ntohl(ip_checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)));
		if (ok) {
			perror("Bad Checksum! Dropping Packet...");
			continue;
		}

		/* TODO 2.2: Call get_best_route to find the most specific route, continue; (drop) if null */
		struct route_table_entry *best_router = get_best_route(ip_hdr->daddr);
		if (!best_router) {
			perror("No route. Dropping Packet...");
			continue;
		}

		/* TODO 2.3: Check TTL > 1. Update TLL. Update checksum  */
		if (ip_hdr->ttl <= 1) {
			perror("TTL expired. Dropping packet...");
			continue;
		}
		ip_hdr->ttl--;
		ip_hdr->check = 0;
		ip_hdr->check = htons(ip_checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)));
		// neaparat trebuie htons nu htonl

		/* TODO 2.4: Update the ethernet addresses. Use get_mac_entry to find the destination MAC
		 * address. Use get_interface_mac(m.interface, uint8_t *mac) to
		 * find the mac address of our interface. */
		struct mac_entry *entry = get_mac_entry(best_router->next_hop);
		if (!entry) {
			perror("Bad Mac. Dropping Packet...");
			continue;
		}
		  
		// Call send_to_link(best_router->interface, packet, packet_len);
		memcpy(eth_hdr->ether_dhost, entry->mac, 6);
		get_interface_mac(best_router->interface, eth_hdr->ether_shost);
		send_to_link(best_router->interface, packet, packet_len);
	}
}
