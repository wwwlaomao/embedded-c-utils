#define ipaddr_len_c 16
/** Size of IPv6 header */
#define IP_IPH_LEN 40
/** Types of Next header (Protocols) */
#define IP_PROTO_UDP 17  /** UDP header */

/** IP address type. 0 for link local address, 1 for global address and 2 for
 multicast address. The most significant byte is saved in byte 0. */
typedef uint8_t ipaddr_t[ipaddr_len_c];

/* The UDP header. */
typedef struct udp_hdr_tag
{
    /** Source port */
    uint8_t srcport[2];
    /** Destination port */
    uint8_t dstport[2];
    /** Length of UDP datagram */
    uint8_t len[2];
    /** Checksum */
    uint8_t chksum[2];
} udp_hdr_t;

/**
Calculate udp packet checksum.
@param[in] hdr The pointer to packet.
 */
uint16_t calc_udp_chksum(ip_hdr_t * hdr)
{
    uint8_t * pos = (uint8_t *)hdr;
    udp_hdr_t * udp = (udp_hdr_t *)(pos + IP_IPH_LEN);
    udp->chksum[0] = 0;
    udp->chksum[1] = 0;

    return calc_upper_layer_chksum(hdr, IP_PROTO_UDP);
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
@param[in] sum The seperated number to be added in the calculation.
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