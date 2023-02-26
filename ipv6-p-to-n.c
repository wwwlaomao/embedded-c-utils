#define ipaddr_len_c 16

/**
Convert IPv6 presentation format to network number.
@param[in] src The source string containing an IPv6 presentation.
@param[in] size Length of the string in bytes.
@param[out] dst The destination of the network number.
@return The length of the prefix in bits.
*/
uint8_t app_pton(uint8_t * src, uint8_t size, uint8_t * dst)
{
    uint8_t outbuf[ipaddr_len_c];
    uint8_t * tp;
    uint8_t * sp;
    uint8_t * endp;
    uint8_t * colonp;
    uint8_t bits = 128;
    uint16_t n;
    uint8_t bytes;
    uint8_t i;

    sp = src;
    /* There are at least two charaters for a valid IPv6 address. */
    if (size < 2)
    {
        bits = 0;
    }
    /* Leading :: requires some special handling. */
    else if (':' == *sp)
    {
        ++sp;
        if (':' != *sp)
        {
            bits = 0;
        }
    }

    if (0 != bits)
    {
        tp = outbuf;
        endp = outbuf + ipaddr_len_c;
        memset(tp, '\0', ipaddr_len_c);
        colonp = NULL;
        bits = app_pton_ipv6(sp, size - (sp - src), &tp, endp, &colonp);
    }

    if (0 != bits)
    {
        if (colonp != NULL)
        {
            n = tp - colonp;
            for (i = 1; i <= n; ++i)
            {
                endp[- i] = colonp[n - i];
                colonp[n - i] = 0;
            }
        }
        /* Apply bits */
        if (bits < 128)
        {
            bytes = (bits + 7) / 8;
            if (bytes < ipaddr_len_c)
            {
                memset(outbuf + bytes, 0, ipaddr_len_c - bytes);
            }
            if (0 != bits % 8)
            {
                outbuf[bytes - 1] &= ((~0) << (8 - (bits % 8)));
            }
        }
        memcpy(dst, outbuf, ipaddr_len_c);
    }

    return (bits);
}

/**
Convert IPv6 presentation format to network number.
@param[in] src The source string containing an IPv6 presentation.
@param[in] size Length of the string in bytes.
@param[in,out] tp The pointer to the output buffer as input and the pointer to
the next byte of the last output byte.
@param[in] endp Next byte of the output buffer.
@param[out] colonp Pointer to the double colon in the string.
@return The length of the prefix in bits. The default value 128 is returned when
there is no prefix.
*/
static uint8_t app_pton_ipv6(
    uint8_t * src, uint8_t size, uint8_t ** tp,
    uint8_t * endp, uint8_t **colonp)
{
    uint8_t * sp = src;
    uint8_t * curtok;
    uint8_t ch;
    bool saw_xdigit;
    uint16_t val;
    uint8_t digits;
    uint8_t * endptr;
    const uint8_t * xdigits;
    const uint8_t * pch;
    uint8_t i;
    uint8_t bits = 128;

    curtok = sp;
    saw_xdigit = false;
    val = 0;
    digits = 0;
    for (i = 0; i < size; ++i)
    {
        ch = *sp;
        ++sp;
        if (ch == '\0')
        {
            break;
        }
        else if (ch == ':')
        {
            if (true == saw_xdigit)
            {
                saw_xdigit = false;
                if (*tp + 2 <= endp)
                {
                    **tp = (uint8_t) (val >> 8) & 0xff;
                    ++*tp;
                    **tp = (uint8_t) val & 0xff;
                    ++*tp;
                }
                else
                {
                    bits = 0;
                    break;
                }
            }
            else
            {
                if (NULL == *colonp)
                {
                    *colonp = *tp;
                }
                else
                {
                    bits = 0;
                    break;
                }
            }
            curtok = sp;
            digits = 0;
            val = 0;
        }
        else if (ch == '.')
        {
            if ((*tp + 4) <= endp)
            {
                bits = app_pton_v4(curtok, size - (curtok - src), *tp);
                if (bits > 0)
                {
                    *tp += 4;
                    saw_xdigit = false;
                }
                break;
            }
            else
            {
                bits = 0;
            }
        }
        else if (ch == '/')
        {
            bits = app_str_to_int32(sp, size - (sp - src), &endptr, base_dec_c);
            break;
        }
        else
        {
            xdigits = mHexDigitsL;
            pch = strchr(xdigits, ch);
            if (NULL == pch)
            {
                xdigits = mHexDigitsU;
                pch = strchr(xdigits, ch);
            }
            if (NULL != pch)
            {
                ++digits;
                if (digits > 4)
                {
                    bits = 0;
                    break;
                }
                val <<= 4;
                val |= (pch - xdigits);
                saw_xdigit = true;
            }
            else
            {
                bits = 0;
                break;
            }
        }
    }

    if (true == saw_xdigit)
    {
        if (*tp + 2 <= endp)
        {
            **tp = (uint8_t) (val >> 8) & 0xff;
            ++*tp;
            **tp = (uint8_t) val & 0xff;
            ++*tp;
        }
        else
        {
            bits = 0;
        }
    }

    return bits;
}

/**
Convert IPv4 presentation format to network number.
@param[in] src The source string containing an IPv4 presentation.
@param[in] size Length of the string in bytes.
@param[out] dst The destination of the network number.
@return The length of the prefix in bits. The default value 128 is returned when
there is no prefix.
*/
static uint8_t app_pton_v4(uint8_t * src, uint8_t size, uint8_t * dst)
{
    uint8_t * tp = dst;
    uint8_t * endp = dst + 4;
    uint8_t * sp = src;
    uint8_t digits = 0;
    uint16_t val = 0;
    uint8_t ch;
    char * pch;
    uint8_t * endptr;
    uint8_t i;
    uint8_t bits = 128;

    for (i = 0; i < size; ++i)
    {
        ch = *sp;
        ++sp;
        if ('\0' == ch)
        {
            break;
        }
        else if ('.' == ch)
        {
            if (tp < endp)
            {
                *tp = val;
                ++tp;
                val = 0;
                digits = 0;
            }
            else
            {
                bits = 0;
                break;
            }
        }
        else if ('/' == ch)
        {
            bits = app_str_to_int32(sp, size - (sp - src), &endptr, base_dec_c);
            break;
        }
        else
        {
            pch = strchr(mDecDigits, ch);
            if (NULL != pch)
            {
                if ((0 != digits) && (0 == val))
                {
                    bits = 0;
                    break;
                }
                ++digits;
                val *= 10;
                val += (pch - mDecDigits);
                if (val > 255)
                {
                    bits = 0;
                    break;
                }
            }
           }
    }

    if (1 != (endp - tp))
    {
        bits = 0;
    }
    else if (0 == digits)
    {
        bits = 0;
    }

    if (0 != bits)
    {
        *tp = val;
    }

    return bits;
}