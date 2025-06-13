#include <stdlib.h>
#include <stdio.h>
#include "mcoder.h"
#include <math.h>
#include <iostream>


/*!
 ************************************************************************
 * \brief
 *    Allocates memory for the DecodingEnvironment struct
 * \return DecodingContextPtr
 *    allocates memory
 ************************************************************************
 */
DecodingEnvironmentPtr arideco_create_decoding_environment() {
    DecodingEnvironmentPtr dep;

    if ((dep = (DecodingEnvironmentPtr) calloc(1, sizeof(DecodingEnvironment))) == NULL)
        printf("arideco_create_decoding_environment: dep");
    return dep;
}


/*!
 ***********************************************************************
 * \brief
 *    Frees memory of the DecodingEnvironment struct
 ***********************************************************************
 */
void arideco_delete_decoding_environment(DecodingEnvironmentPtr dep) {
    free(dep);
}


/*!
 ************************************************************************
 * \brief
 *    Initializes the DecodingEnvironment for the arithmetic coder
 ************************************************************************
 */
void arideco_start_decoding(DecodingEnvironmentPtr dep, unsigned char *cpixcode,
                            int firstbyte, int *cpixcode_len) {
    unsigned int bbit = 0;

    long long value = 0;
    dep->Dcodestrm = cpixcode;
    dep->Dcodestrm_len = (unsigned int *) cpixcode_len;
    dep->Dvalue = 0;
    for (int i = 1; i <= BITS_IN_REGISTER; i++) {
        Get1Bit(dep->Dcodestrm, *dep->Dcodestrm_len, bbit);
        dep->Dvalue = 2 * dep->Dvalue + bbit;
    }
    dep->Dlow = 0;
    dep->Drange = HALF - 2;
}

unsigned int biari_decode_symbol(DecodingEnvironmentPtr dep, BiContextTypePtr bi_ct) {
    int bbit;
    unsigned long long low = dep->Dlow;
    unsigned long long value = dep->Dvalue;
    unsigned long long range = dep->Drange;
    unsigned long long symbol = 256;

    unsigned long long cum = ((value - low + 1) * bi_ct->freq_all - 1) / range;
    for (int i = 0; i <= ALPHABET_SIZE; i++) {
        if (bi_ct->cum_freq[i] > cum) {
            break;
        }
        symbol = i;
    }
    // std::cout << bi_ct->name << " " << symbol << " " << (((symbol) == 256) ? "esc" : std::string(1, (unsigned char)symbol)) << " " << bi_ct->freq[symbol] << " " << bi_ct->freq_all << " " << low << " " << range << std::endl;
    // for (int i = 0 ; i <= ALPHABET_SIZE; i++) {
    //     std::cout << bi_ct->cum_freq[i] << " ";
    // }
    // std::cout << std::endl;

    low = low + (range * bi_ct->cum_freq[symbol]) / bi_ct->freq_all;
    range = (range * bi_ct->freq[symbol]) / bi_ct->freq_all;
    range = max_val(1, range);

    // printf("DEC: low=%u, range=%u, value=%u before renorm\n", low, range, value);
    while (range < QUARTER) {
        if (low >= HALF) {
            low -= HALF;
            value -= HALF;
        } else if (low < QUARTER) {
        } else {
            low -= QUARTER;
            value -= QUARTER;
        }
        // Double range
        range <<= 1;
        low <<= 1;
        Get1Bit(dep->Dcodestrm, *dep->Dcodestrm_len, bbit);
        value = (value << 1) | bbit;
    }

    dep->Drange = range;
    dep->Dvalue = value;
    dep->Dlow = low;
    // printf("DEC: low=%u, range=%u, value=%u after renorm\n", low, range, value);
    return (symbol);
}

unsigned int biari_peek_symbol(DecodingEnvironmentPtr dep, BiContextType *ctx) {

    unsigned int low = dep->Dlow;
    unsigned int value = dep->Dvalue;
    unsigned int range = dep->Drange;
    unsigned int cum = ((value - low + 1) * ctx->freq_all - 1) / range;


    for (int symbol = 0; symbol <= ALPHABET_SIZE; ++symbol) {
        if (cum < ctx->cum_freq[symbol]) {
            return symbol;
        }
    }


    return ALPHABET_SIZE;
}
