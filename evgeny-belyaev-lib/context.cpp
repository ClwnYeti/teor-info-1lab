#include <iostream>
#include <string>

#include "mcoder.h"

void biari_init_context
(
    BiContextTypePtr ctx,
    std::string name
) {
    ctx->name = name;

    ctx->freq_all = 0;

    for (int i = 0; i <= ALPHABET_SIZE; i++) {
        ctx->freq[i] = 0;
    }

    biari_calculate_context(ctx);
}

void biari_init_unique_context
(
    BiContextTypePtr ctx,
    std::string name
) {
    ctx->name = name;

    ctx->freq_all = 0;

    for (int i = 0; i <= ALPHABET_SIZE; i++) {
        if (i != ESC_SYMBOL) {
            ctx->freq[i] = 1;
        } else {
            ctx->freq[i] = 0;
        }
        ctx->freq_all += ctx->freq[i];
    }
    biari_calculate_context(ctx);
}

void biari_update_context
(
    BiContextTypePtr ctx,
    int symbol
) {
    if (ctx->freq_all > 16384) {
        ctx->freq_all = 0;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            ctx->freq[i] = max_val(1, ctx->freq[i]>>1);
            ctx->freq_all += ctx->freq[i];
        }
    }

    ctx->freq[symbol]++;
    ctx->freq_all++;

    biari_calculate_context(ctx);
    // printf("Context %s was updated with symbol %d\n", ctx->name.c_str(), symbol);
}

void biari_calculate_context
(
    BiContextTypePtr ctx
) {
    ctx->cum_freq[0] = 0;
    for (int i = 1; i <= ALPHABET_SIZE; i++) {
        ctx->cum_freq[i] = ctx->cum_freq[i - 1] + ctx->freq[i - 1];
    }
}

void biari_reset_update_context
(
    BiContextTypePtr ctx,
    int symbol
) {
    ctx->freq[symbol]--;
    ctx->freq_all--;

    biari_calculate_context(ctx);
    // printf("Context %s had a reset of update with symbol %d\n", ctx->name.c_str(), symbol);
}