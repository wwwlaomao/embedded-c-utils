#define ipaddr_len_c 16
/** Size of IPv6 header */
#define IP_IPH_LEN    			40
/** Types of Next header (Protocols) */
#define IP_PROTO_ICMP6 	            58  /** ICMPv6 header */

/** IP address type. 0 for link local address, 1 for global address and 2 for
 multicast address. The most significant byte is saved in byte 0. */
typedef uint8_t ipaddr_t[ipaddr_len_c];

/** The IPv6 header. */
typedef struct ip_hdr_tag
{
    /** Version + Traffic class  */
    uint8_t vtc;
    /** Traffic class + Flow label */
    uint8_t tcflow;
    /** Flow label 16bits */
    uint8_t flow[2];
    /** Length of the IPv6 payload */
    uint8_t len[2];
    /** Type of header immediately following the IPv6 header. */
    uint8_t proto;
    /** Hop limit - Time to Live*/
	uint8_t ttl;
    /** 128-bit address of the originator of the packet. */
    ipaddr_t srcaddr;
    /** 128-bit address of the intended recipient of the packet */
    ipaddr_t dstaddr;	
} ip_hdr_t;

/** The ICMPv6 header. */
typedef struct icmp_hdr_tag
{
    /** Type of ICMP message */
    uint8_t type;
    /** Code */
    uint8_t code;
    /** Checksum */
    uint8_t chksum[2];
} icmp_hdr_t;

/**
Calculate icmp packet checksum.
@param[in] hdr The pointer to packet.
 */
uint16_t calc_icmp_chksum(ip_hdr_t * hdr)
{
    uint8_t * pos = (uint8_t *)hdr;
    icmp_hdr_t * icmp = (icmp_hdr_t *)(pos + IP_IPH_LEN);
    icmp->chksum[0] = 0;
    icmp->chksum[1] = 0;

    return calc_upper_layer_chksum(hdr, IP_PROTO_ICMP6);
}

/**
Calculate upper layer checksum according to rfc 2460 section 8.1.
@param[in] hdr The ip header of the packet to be calculated.
@param[in] proto The upper layer protocol.
@return The upper layer checksum in host byte order.
*/
static uint16_t calc_upper_layer_chksum(ip_hdr_t * hdr, uint8_t proto)
{
    uint16_t sum = 0;
    uint16_t upper_layer_len;
    uint8_t * ptr = (uint8_t *)hdr;

    upper_layer_len = ((hdr->len[0]) << 8) + hdr->len[1];

    /* First sum pseudoheader. */
    sum = upper_layer_len + proto;
    sum = calc_sum(sum, (uint8_t *)&hdr->srcaddr, 2 * ipaddr_len_c);

    /* Sum upper layer header and data. */
    sum = calc_sum(sum, (uint8_t *)(ptr + IP_IPH_LEN), upper_layer_len);

    return(0xffff - sum);
}

/**
Calculate sum of a series of data.
@param[in] sum The separated number to be added in the calculation.
@param[in] data The pointer to the data.
@param[in] len The length of the data in bytes.
@return The checksum in host byte order.
*/
static uint16_t calc_sum(uint16_t sum, const uint8_t *data, uint16_t len)
{
    uint16_t t;
    const uint8_t *dataptr;
    const uint8_t *last_byte;

    dataptr = data;
    last_byte = data + len - 1;

    while(dataptr < last_byte)
    {
        /* At least two more bytes */
        t = (dataptr[0] << 8) + dataptr[1];
        sum += t;
        if (sum < t)
        {
            ++sum; /* Carry */
        }
        dataptr += 2;
    }

    if (dataptr == last_byte)
    {
        t = (dataptr[0] << 8) + 0;
        sum += t;
        if (sum < t)
        {
            ++sum; /* carry */
        }
    }
    return sum;
}