/**
Decode an ASCII array to a hex array.
@param[in] ascii A pointer to an ASCII array, which are either small or capital
letters.
@param[in] len The length of the input ASCII array in bytes. When the length is
odd, a zero is added before the first byte, "ABC" is effectively same as "0ABC".
@param[out] hex A pointer to a decoded hex/bcd array. The memory should be
allocated outside and its size is half of the input array.
@param[out] endptr The position to the first byte that not used in the
conversion.
@return The length of the decoded "hex" array. When parameter "hex" is NULL,
the memory size requested by the decoded "hex" array is returned.
*/
uint16_t app_ascii_to_hex(uint8_t * hex, uint8_t * ascii,
                          uint16_t len, uint8_t ** endptr)
{
    uint8_t ch;
    uint16_t i = 0;
    uint8_t digits = 0;
    uint8_t offset = 0;
    uint8_t half = 0;

    *endptr = ascii;
    if (NULL != ascii)
    {
        if ((len & 0x01) != 0)
        {
            i = 1;
            ++len;
            offset = 1;
        }
        else
        {
            i = 0;
        }

        for (; i < len; ++i, ++ascii)
        {
            ch = *ascii;
            if ((ch >= 'A') && (ch <= 'F'))
            {
                ch -= 'A' - 0x0A;
            }
            else if ((ch >= 'a') && (ch <= 'f'))
            {
                ch -= 'a' - 0x0a;
            }
            else if ((ch >= '0') && (ch <= '9'))
            {
                ch -= '0';
            }
            else
            {
                break;
            }

            if ((i & 0x01) == 0)
            {
                digits = ch;
                half = 1;
            }
            else
            {
                if (NULL != hex)
                {
                    hex[i >> 1] = (digits << 4) + ch;
                }
                half = 0;
            }
        }
    }

    *endptr += (i - offset - half);

    return(i >> 1);
}