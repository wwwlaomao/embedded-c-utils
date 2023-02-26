/**
This function has same functionality as strtol() except that it handles string
without depending on the terminating \0 but length.
@param[in] s The initial string to convert.
@param[in] len The length of the string.
@param[out] endptr It stores a pointer to the unconverted remainder of the
string or NULL when there is no unconverted remainder.
@param[in] base Base of the integer, in the range of [2, 36].
@return The integer converted from the string. 0 is returned when the string
does not match a valid pattern.
*/
int32_t app_str_to_int32(uint8_t * s,
                         uint8_t len,
                         uint8_t ** endptr,
                         app_stol_base_t base)
{
    /* Maximum 32 characters when base is 2
    plus one byte for sign and one byte for \0 */
    uint8_t str[34] = {0};
    int32_t ret;

    memcpy(str, s, len);
    ret = strtol((int8_t*)str, endptr, base);
    *endptr = s + (*endptr - str);
    return ret;
}