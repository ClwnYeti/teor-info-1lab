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
    ctx->freq[ALPHABET_SIZE] = 0;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        ctx->freq[i] = 1;
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
    printf("Context %s was updated with symbol %d\n", ctx->name.c_str(), symbol);
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
    printf("Context %s had a reset of update with symbol %d\n", ctx->name.c_str(), symbol);
}

void biari_calculate_with_excluded_symbols_context
(
    BiContextTypePtr ctx,
    std::unordered_set<unsigned int> &symbols
) {
    for (unsigned int symbol: symbols) {
        ctx->freq_all -= ctx->freq[symbol];
    }
    ctx->cum_freq[0] = 0;
    unsigned int last_freq_with_value = 0;
    if (!symbols.contains(0)) {
        last_freq_with_value = ctx->freq[0];
    }
    for (int i = 1; i <= ALPHABET_SIZE; i++) {
        if (!symbols.contains(i)) {
            ctx->cum_freq[i] = ctx->cum_freq[i - 1] + last_freq_with_value;
            last_freq_with_value = ctx->freq[i];
        } else {
            ctx->cum_freq[i] = ctx->cum_freq[i - 1];
        }
    }

    std::cout << "For context " << ctx->name << " were excluded symbols: ";
    for (auto s: symbols) {
        std::cout << s << " ";
    }
    std::cout << "\n";
}

void biari_return_context_normal_state
(
    BiContextTypePtr ctx
) {
    ctx->freq_all = 0;
    for (int i = 0; i <= ALPHABET_SIZE; i++) {
        ctx->freq_all += ctx->freq[i];
    }

    biari_calculate_context(ctx);
    std::cout << "Context " << ctx->name << " was restored\n";
}