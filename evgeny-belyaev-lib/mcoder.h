#ifndef MCODER_H
#define MCODER_H

#include "stream.h"
#include <stdio.h>
#include <string>
#include <unordered_set>

//new parameters

#define BITS_IN_REGISTER 16
#define TOP_VALUE (((long) 1 << BITS_IN_REGISTER) - 1)
#define QUARTER (TOP_VALUE / 4 + 1)
#define FIRST_QTR (TOP_VALUE / 4 + 1)
#define HALF      (2 * FIRST_QTR)
#define THIRD_QTR (3 * FIRST_QTR)

#define ALPHABET_SIZE 256
#define max_val(a,b) (((a) > (b)) ? (a) : (b))

typedef struct {
    unsigned long long freq[ALPHABET_SIZE + 1];
    unsigned long long freq_all;
    unsigned long long cum_freq[ALPHABET_SIZE + 1];
    unsigned long long freq_stat[ALPHABET_SIZE + 1];
    std::string name;
} BiContextType;

typedef BiContextType *BiContextTypePtr;

#define putLogToFile(fname,...) { \
  FILE *fp = fopen(fname,"at"); \
  fprintf(fp,__VA_ARGS__); \
  fclose(fp); \
}


#define SIGN_TO_ENCODE(x) ((x) >= 0 ? 0 : 1)
#define CABAC_ABS(x) ((x) > 0 ? (x) : -(x))
#define MAX_BITS_IN_SERIE 25
///   25 MAXIMUM
#define PutZeros(nbits) {\
    unsigned int iii;\
    for(iii = 0; iii<(nbits); iii++ )\
    {\
      Put1Bit(eep->Ecodestrm, *eep->Ecodestrm_len, 0);\
    }\
      }

#define PutLongOnes(nbits) {\
      unsigned int i1=0xFFFFFFFF;\
      int bits1;\
      int main = (nbits)/MAX_BITS_IN_SERIE;\
      int tail = (nbits)%MAX_BITS_IN_SERIE;\
      for(bits1 = 0; bits1<main; bits1++ ) {\
        PutBits(eep->Ecodestrm, *eep->Ecodestrm_len,i1,MAX_BITS_IN_SERIE);\
      }\
      PutBits(eep->Ecodestrm, *eep->Ecodestrm_len,i1,tail);\
    }

#define put_one_bit_0_plus_outstanding { \
Put1Bit(eep->Ecodestrm, *eep->Ecodestrm_len, 0); \
PutOnes(eep->Ebits_to_follow); \
eep->Ebits_to_follow = 0; \
}

#define put_one_bit_1_plus_outstanding { \
Put1Bit(eep->Ecodestrm, *eep->Ecodestrm_len, 1); \
PutZeros(eep->Ebits_to_follow); \
eep->Ebits_to_follow = 0; \
}


//! struct to characterize the state of the arithmetic coding engine
typedef struct {
    unsigned long long Elow, Erange, Ehigh;
    unsigned long long Ebits_to_follow;
    unsigned char *Ecodestrm;
    unsigned int *Ecodestrm_len;
} EncodingEnvironment;

typedef EncodingEnvironment *EncodingEnvironmentPtr;

// cabac_enc.c
void biari_init_context
(
    BiContextTypePtr ctx,
    std::string name
);

void biari_init_unique_context
(
    BiContextTypePtr ctx,
    std::string name
);

void biari_update_context
(
    BiContextTypePtr ctx,
    int symbol
);

void biari_reset_update_context
(
    BiContextTypePtr ctx,
    int symbol
);

void biari_calculate_context
(
    BiContextTypePtr ctx
);


EncodingEnvironmentPtr arienco_create_encoding_environment();

void arienco_delete_encoding_environment(EncodingEnvironmentPtr eep);

void arienco_start_encoding(EncodingEnvironmentPtr eep,
                            unsigned char *code_buffer,
                            int *code_len);

void arienco_done_encoding(EncodingEnvironmentPtr eep);

void biari_encode_symbol(EncodingEnvironmentPtr eep, int symbol, BiContextTypePtr bi_ct);


//! struct to characterize the state of the arithmetic coding engine
typedef struct {
    unsigned long long Dlow, Drange, Dhigh;
    unsigned long long Dvalue;
    unsigned long long Dbuffer;
    unsigned long long Dbits_to_go;
    unsigned char *Dcodestrm;
    unsigned int *Dcodestrm_len;
} DecodingEnvironment;

typedef DecodingEnvironment *DecodingEnvironmentPtr;

// cabac_dec.c
DecodingEnvironmentPtr arideco_create_decoding_environment();

void arideco_delete_decoding_environment(DecodingEnvironmentPtr dep);

void arideco_start_decoding(DecodingEnvironmentPtr dep, unsigned char *cpixcode,
                            int firstbyte, int *cpixcode_len);

int arideco_bits_read(DecodingEnvironmentPtr dep);

void arideco_done_decoding(DecodingEnvironmentPtr dep);

unsigned int biari_decode_symbol(DecodingEnvironmentPtr dep, BiContextTypePtr bi_ct);

unsigned int biari_peek_symbol(DecodingEnvironmentPtr dep, BiContextType *ctx);
#endif // !MCODER_H
