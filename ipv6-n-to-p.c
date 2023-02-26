#define ipaddr_len_c 16

/**
Convert IPv6 network number from network to presentation format compliant to
ietf-rfc 4291 with update in rfc 5952.
Generates CIDR style result always.
Picks the shortest representation unless the IP is really IPv4.
Always prints specified number of bits (bits).
Network byte order assumed which means 192.5.5.240/28 has b11110000
in its fourth octet.
@param[in] src The array containing IPv6 network number.
@param[in] bits Number of bits of the address.
@param[out] dst The destination of the presentation output.
The buffer is untouched when the conversion is not successful.
@param[in] size Size of the destination buffer.
@return Length of the string written to the destination buffer in bytes.
*/
uint8_t app_ntop(uint8_t * src, uint8_t bits, uint8_t * dst, uint8_t size)
{
    uint8_t inbuf[ipaddr_len_c];
    uint8_t outbuf[sizeof("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:255.255.255.255/128")];
    uint8_t bytes;
    uint8_t words;
    uint8_t best_pos;
    uint8_t best_len;
    bool is_ipv4 = false;
    uint8_t * dp;
    uint8_t len = 0;

    if (bits <= 128)
    {
        dp = outbuf;
        if (bits == 0)
        {
            *dp = ':';
            ++dp;
            *dp = ':';
            ++dp;
            *dp = '\0';
        }
        else
        {
            /* Copy src to private buffer.  Zero host part. */
            bytes = (bits + 7) / 8;
            memcpy(inbuf, src, bytes);
            memset(inbuf + bytes, 0, ipaddr_len_c - bytes);
            if (0 != (bits % 8))
            {
                inbuf[bytes - 1] &= (~0 << (8 - (bits % 8)));
            }

            /* how many words need to be displayed in output */
            words = (bits + 15) / ipaddr_len_c;
            if (words == 1)
            {
                words = 2;
            }

            best_len = app_ntop_longest_zeros(inbuf, words, &best_pos);
            is_ipv4 = app_ntop_is_ipv4(inbuf, words, best_pos, &best_len);
            app_ntop_format(inbuf, words, bits,
                            best_pos, best_len, is_ipv4, dp);
        }

        if (strlen(outbuf) < size)
        {
            strcpy(dst, outbuf);
            len = strlen(outbuf);
        }
       }

    return len;
}

/**
Find out the substring of the longest continuous zeros.
@param[in] src The pointer to the buffer of the string.
@param[in] words The number of words to be displayed.
@param[out] pos The start position of the substring of the longest continuous
zeros.
@return Length of the substring of the longest continuous zeros.
*/
static uint8_t app_ntop_longest_zeros(uint8_t * src,
                                      uint8_t words,
                                      uint8_t * pos)
{
    uint8_t cur_pos = 0;
    uint8_t cur_len = 0;
    uint8_t best_pos = 0;
    uint8_t best_len = 0;
      uint8_t i;

    for (i = 0; i < (words * 2); i += 2)
    {
        if ((0 == src[i]) && (0 == src[i+1]))
        {
            if (cur_len == 0)
            {
                cur_pos = (i / 2);
            }
            ++cur_len;
        }
        else
        {
            if (best_len < cur_len)
            {
                best_pos = cur_pos;
                best_len = cur_len;
                cur_len = 0;
            }
        }
    }

    if (best_len < cur_len)
    {
        best_pos = cur_pos;
        best_len = cur_len;
    }

    *pos = best_pos;
    return best_len;
}

/**
Check whether the string contains an IPv4 address.
@param[in] src The pointer to the buffer of the string.
@param[in] words The number of words to be displayed.
@param[in] best_pos The start position of the substring of the longest
continuous zeros.
@param[in,out] best_len Length of the substring of the longest continuous zeros.
@return True when the string contains an IPv4 address or false otherwise.
*/
static bool app_ntop_is_ipv4(uint8_t * src, uint8_t words, uint8_t best_pos,
                             uint8_t * best_len)
{
    bool is_ipv4 = false;

    if ((*best_len != words) && (0 == best_pos))
    {
        if (*best_len == 6)
        {
            is_ipv4 = true;
        }
        else if ((*best_len == 5) && (src[10] == 0xff) && (src[11] == 0xff))
        {
            is_ipv4 = true;
        }
        else if ((*best_len == 7) && ((src[14] != 0) || (src[15] != 1)))
        {
            is_ipv4 = true;
            *best_len = 6;
        }
    }

    return is_ipv4;
}

/**
Format the output for IP address.
@param[in] src The pointer to the buffer of the string.
@param[in] words The number of words to be displayed.
@param[in] bits The number of bits of the address.
@param[in] best_pos The start position of the substring of the longest
continuous zeros.
@param[in] best_len Length of the substring of the longest continuous zeros.
@param[in] is_ipv4 Whether the address is an IPv4 address.
@param[out] dst The buffer for output.
@return The length of the output string.
*/
static uint8_t app_ntop_format(uint8_t * src, uint8_t words, uint8_t bits,
                               uint8_t best_pos, uint8_t best_len, bool is_ipv4,
                               uint8_t * dst)
{
    uint8_t i;
    uint8_t * dp = dst;

    for (i = 0; i < words; ++i)
    {
        if ((best_len > 1) && (i >= best_pos) && (i < (best_pos + best_len)))
        {
            /* Time to skip some zeros */
            if (i == best_pos)
            {
                *dp = ':';
                ++dp;
            }
            if (i == words - 1)
            {
                *dp = ':';
                ++dp;
            }
            src += 2;
            continue;
        }

        if ((true == is_ipv4) && (i > 5))
        {
            *dp = (i == 6) ? ':' : '.';
            ++dp;
            dp += sprintf(dp, "%u", *src);
            ++src;
            /* we can potentially drop the last octet */
            if (i != 7 || bits > 120)
            {
                *dp = '.';
                ++dp;
                dp += sprintf(dp, "%u", *src);
                ++src;
            }
        }
        else
        {
            if (dp != dst)
            {
                *dp = ':';
                ++dp;
            }
            dp += sprintf(dp, "%x", (((*src) << 8) + src[1]));
            src += 2;
        }
    }
    if (bits != 128)
    {
        /* Format CIDR /width. */
        sprintf(dp, "/%u", bits);
    }
    else
    {
        *dp = '\0';
    }
    return strlen(dst);
}