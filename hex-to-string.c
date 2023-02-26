/** Digits for hexadecimal in upper case */
static const char mHexDigitsU[] = "0123456789ABCDEF";

/**
Encode a hex/bcd array to an ASCII array.
The number order is first in first out.
@param[in] hex A pointer to an hex/bcd array.
@param[in] len The length of the input hex array in bytes.
Len must not bigger than 32767 (0x7FFF).
@param[out] ascii A pointer to an array stored the output ASCII data, which all
are capital letter only. The memory is allocated by caller and its size shall
be the double as the input data.
@return The length of the decoded "ascii" array. When parameter "ascii" is NULL,
the memory size requested by the decoded "ascii" array is returned.
*/
uint16_t app_hex_to_ascii(uint8_t * ascii, uint8_t * hex, uint16_t len)
{
    uint16_t i;

    if (hex != NULL)
    {
        if (ascii != NULL)
        {
            for (i = 0; i < len; ++i)
            {
                ascii[i << 1] = mHexDigitsU[(hex[i] >> 4) & 0x0F];
                ascii[(i << 1) + 1] = mHexDigitsU[hex[i] & 0x0F];
            }
        }
    }

    return (len << 1);
}