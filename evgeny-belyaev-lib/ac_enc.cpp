#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <math.h>
#include "mcoder.h"

/*!
 ************************************************************************
 * \brief
 *    Initializes a given context with some pre-defined probability state
 ************************************************************************
 */

/*!
 ************************************************************************
 * \brief
 *    Allocates memory for the EncodingEnvironment struct
 ************************************************************************
 */
EncodingEnvironmentPtr arienco_create_encoding_environment() {
    EncodingEnvironmentPtr eep;

    if ((eep = (EncodingEnvironmentPtr) calloc(1, sizeof(EncodingEnvironment))) == NULL)
        printf("arienco_create_encoding_environment: eep");

    return eep;
}

/*!
 ************************************************************************
 * \brief
 *    Frees memory of the EncodingEnvironment struct
 ************************************************************************
 */
void arienco_delete_encoding_environment(EncodingEnvironmentPtr eep) {
    if (eep != NULL) {
        free(eep);
    }
}

/*!
 ************************************************************************
 * \brief
 *    Initializes the EncodingEnvironment for the arithmetic coder
 ************************************************************************
 */
void arienco_start_encoding(EncodingEnvironmentPtr eep,
                            unsigned char *code_buffer,
                            int *code_len) {
    eep->Elow = 0;
    eep->Erange = HALF - 2;
    eep->Ebits_to_follow = 0;
    eep->Ecodestrm = code_buffer;
    eep->Ecodestrm_len = (unsigned int *) code_len;
}

/*!
 ************************************************************************
 * \brief
 *    Terminates the arithmetic codeword, writes stop bit and stuffing bytes (if any)
 ************************************************************************
 */

void arienco_done_encoding(EncodingEnvironmentPtr eep) {
    if ((eep->Elow >> (BITS_IN_REGISTER - 1)) & 1) {
        put_one_bit_1_plus_outstanding;
    } else {
        put_one_bit_0_plus_outstanding;
    }


    // Then flush remaining significant bits from Elow (to distinguish the final range)
    for (int i = 1; i < BITS_IN_REGISTER - 1; ++i) {
        unsigned int bit = (eep->Elow >> (BITS_IN_REGISTER - 1 - i)) & 1;
        Put1Bit(eep->Ecodestrm, *eep->Ecodestrm_len, bit);
    }

    // Optionally: align to byte boundary (pad with 0s)
    *eep->Ecodestrm_len = (*eep->Ecodestrm_len + 7) & ~7;
}


void biari_encode_symbol(EncodingEnvironmentPtr eep, signed int symbol, BiContextTypePtr bi_ct) {
    unsigned long long range = eep->Erange;
    unsigned long long low = eep->Elow;

    // std::cout << bi_ct->name << " " << symbol << " " << (((symbol) == 256) ? "esc" : std::string(1, (unsigned char)symbol)) << " " << bi_ct->freq[symbol] << " " << bi_ct->freq_all << " " << low << " " << range << std::endl;
    // for (int i = 0 ; i <= ALPHABET_SIZE; i++) {
    //     std::cout << bi_ct->cum_freq[i] << " ";
    // }
    // std::cout << std::endl;

    low = low + (range * bi_ct->cum_freq[symbol]) / (bi_ct->freq_all);
    range = (range * bi_ct->freq[symbol]) / bi_ct->freq_all;
    range = max_val(1, range);

    // printf("ENC: low=%u, range=%u before renorm\n", low, range);
    // renormalization
    while (range < QUARTER) {
        if (low >= HALF) {
            put_one_bit_1_plus_outstanding;
            low -= HALF;
        } else if (low < QUARTER) {
            put_one_bit_0_plus_outstanding;
        } else {
            eep->Ebits_to_follow++;
            low -= QUARTER;
        }
        low <<= 1;
        range <<= 1;
    }

    eep->Erange = range;
    eep->Elow = low;
    // printf("ENC: low=%u, range=%u after renorm\n", low, range);
}
