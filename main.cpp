#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "evgeny-belyaev-lib/mcoder.h"

#define UNIQUE_CONTEXT_HASH (-2ll)
#define DEFAULT_CONTEXT_HASH (-1ll)

void step_prefix(long long i) {
//    std::cout << i + 1 << " -> ";
}

struct AlgorithmMetaInfo {
    std::string input_file_name;
    std::string output_file_name;
    bool isEncoder = false;

    AlgorithmMetaInfo() {
    }
};

class CommandLineParser {
public:
    CommandLineParser() = default;

    static void parseArgs(AlgorithmMetaInfo &out, int argc, char **argv);
};

struct FileInfo {
    std::vector<unsigned char> data = std::vector<unsigned char>();

    FileInfo() {
    }
};

void CommandLineParser::parseArgs(AlgorithmMetaInfo &out, const int argc, char **argv) {
    std::string input_file_name;
    bool isEncoder = true;
    std::string output_file_name;
    double coef_value = 0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--input") {
            input_file_name = argv[i + 1];
            i++;
        } else if (arg == "--output") {
            output_file_name = argv[i + 1];
            i++;
        } else if (arg == "decoder") {
            isEncoder = false;
        } else if (arg == "encoder") {
            isEncoder = true;
        }
    }


    out.input_file_name = input_file_name;
    out.output_file_name = output_file_name;
    out.isEncoder = isEncoder;
}

class FileReader {
public:
    FileReader() = default;

    static int read(FileInfo &out, const std::string &input_file_name) {
        std::ifstream file(input_file_name.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "File not found" << std::endl;
            return 1;
        }

        file.seekg(0, std::ios::end);
        out.data.resize(file.tellg());
        file.seekg(0, std::ios::beg);

        file.read(reinterpret_cast<std::istream::char_type *>(out.data.data()), out.data.size());
        file.close();

        return 0;
    }
};

class HashWorker {
public:
    static long long hashFromString(const std::string &str) {
        long long result = 0;
        long long multiplier = 1;
        for (int i = str.size() - 1; i >= 0; i--) {
            unsigned char c = str[i];
            result += multiplier * c;
            multiplier = multiplier * ALPHABET_SIZE;
        }

        return result;
    }

    static std::string stringFromHash(const long long hash) {
        if (hash == -1) {
            return "#EmptyDefaultContext";
        }
        if (hash == -2) {
            return "#EmptyUniqueContext";
        }
        std::string result;
        long long currentMultiplier = 1;
        long long temp = hash;
        while (hash > currentMultiplier * ALPHABET_SIZE) {
            currentMultiplier *= ALPHABET_SIZE;
        }
        while (currentMultiplier > 0) {
            result.append(1, temp / currentMultiplier);
            temp = temp % currentMultiplier;
            currentMultiplier = currentMultiplier / ALPHABET_SIZE;
        }

        return result;
    }

    static long long removeFirstSymbolFromHash(const long long hash) {
        long long currentMultiplier = ALPHABET_SIZE;
        while (hash > currentMultiplier * ALPHABET_SIZE) {
            currentMultiplier *= ALPHABET_SIZE;
        }
        return hash % currentMultiplier;
    }

    static int numOfSymbols(const long long hash) {
        long long currentMultiplier = 1;
        int result = 1;
        while (hash > currentMultiplier * ALPHABET_SIZE) {
            currentMultiplier *= ALPHABET_SIZE;
            result++;
        }
        return result;
    }
};

class PPMAEncoder {
public:
    int D = 5;
    std::unordered_map<long long, BiContextType> contexts;

    PPMAEncoder() = default;

    explicit PPMAEncoder(const int d) {
        D = d;
    }
    

    int encode(std::vector<unsigned char> &in, std::ofstream &out) {
        int code_len = 0;
        EncodingEnvironmentPtr eep;
        std::vector<unsigned char> encoded_data;
        encoded_data.resize(in.size() * 8);
        unsigned char *bufptr = encoded_data.data();

        eep = arienco_create_encoding_environment();
        arienco_start_encoding(eep, bufptr, &code_len);

        contexts.clear();

        contexts[UNIQUE_CONTEXT_HASH] = BiContextType();
        biari_init_unique_context(&contexts[UNIQUE_CONTEXT_HASH], "#EmptyUniqueContext");
        contexts[DEFAULT_CONTEXT_HASH] = BiContextType();
        biari_init_context(&contexts[DEFAULT_CONTEXT_HASH], "#EmptyDefaultContext");
        std::string currentContext;

        for (long long i = 0; i < in.size(); i++) {
            std::vector<long long> contextHashes;
            int d = std::min(D, (int) currentContext.size());
            long long currentHash = HashWorker::hashFromString(currentContext);

            for (int depth = d; depth > 0; depth--) {
                contextHashes.push_back(currentHash);
                currentHash = HashWorker::removeFirstSymbolFromHash(currentHash);
            }

            bool symbolEncoded = false;
            for (long long ctxHash: contextHashes) {
                if (!contexts.contains(ctxHash)) {
                    contexts[ctxHash] = BiContextType();
                    biari_init_context(&contexts[ctxHash], HashWorker::stringFromHash(ctxHash));
                }

                biari_update_context(&contexts[ctxHash], ESC_SYMBOL);
                if (contexts[ctxHash].freq[in[i]] > 0) {
                    step_prefix(i);
                    biari_encode_symbol(eep, in[i], &contexts[ctxHash]);
                    biari_reset_update_context(&contexts[ctxHash], ESC_SYMBOL);
                    symbolEncoded = true;
                    break;
                }

                step_prefix(i);
                biari_encode_symbol(eep, ESC_SYMBOL, &contexts[ctxHash]);
                biari_reset_update_context(&contexts[ctxHash], ESC_SYMBOL);
            }

            if (!symbolEncoded) {
                biari_update_context(&contexts[DEFAULT_CONTEXT_HASH], ESC_SYMBOL);

                if (contexts[UNIQUE_CONTEXT_HASH].freq[in[i]] == 1) {
                    step_prefix(i);
                    biari_encode_symbol(eep, ESC_SYMBOL, &contexts[DEFAULT_CONTEXT_HASH]);

                    step_prefix(i);
                    biari_encode_symbol(eep, in[i], &contexts[UNIQUE_CONTEXT_HASH]);
                    biari_reset_update_context(&contexts[UNIQUE_CONTEXT_HASH], in[i]);
                } else {
                    step_prefix(i);
                    biari_encode_symbol(eep, in[i], &contexts[DEFAULT_CONTEXT_HASH]);
                }

                biari_reset_update_context(&contexts[DEFAULT_CONTEXT_HASH], ESC_SYMBOL);
            }

            long long updateHash = HashWorker::hashFromString(currentContext);
            for (int depth = std::min(D, (int) currentContext.size()); depth > 0; depth--) {
                biari_update_context(&contexts[updateHash], in[i]);
                // std::cout << contexts[updateHash].name << std::endl;
                // for (int j = 0; j <= ALPHABET_SIZE; j++) {
                //     // std::cout << j << "\t";
                // }
                // std::cout << std::endl;
                // for (int j = 0; j <= ALPHABET_SIZE; j++) {
                //     // std::cout << contexts[updateHash].cum_freq[j] << "\t";
                // }
                // std::cout << std::endl;
                updateHash = HashWorker::removeFirstSymbolFromHash(updateHash);
            }

            biari_update_context(&contexts[DEFAULT_CONTEXT_HASH], in[i]);

            currentContext.push_back(in[i]);
            if (currentContext.size() > D)
                currentContext = currentContext.substr(currentContext.size() - D);
        }

        std::vector<long long> contextHashes;

        int d = std::min(D, (int) currentContext.size());
        long long currentHash = HashWorker::hashFromString(currentContext);

        for (int depth = d; depth > 0; depth--) {
            contextHashes.push_back(currentHash);
            currentHash = HashWorker::removeFirstSymbolFromHash(currentHash);
        }
        for (long long ctxHash: contextHashes) {
            if (!contexts.contains(ctxHash)) {
                contexts[ctxHash] = BiContextType();
                biari_init_context(&contexts[ctxHash], HashWorker::stringFromHash(ctxHash));
            }

            biari_update_context(&contexts[ctxHash], ESC_SYMBOL);

            // std::cout << "EOF -> ";
            biari_encode_symbol(eep, ESC_SYMBOL, &contexts[ctxHash]);
            biari_reset_update_context(&contexts[ctxHash], ESC_SYMBOL);
        }
        biari_update_context(&contexts[DEFAULT_CONTEXT_HASH], ESC_SYMBOL);
        biari_encode_symbol(eep, ESC_SYMBOL, &contexts[DEFAULT_CONTEXT_HASH]);
        biari_encode_symbol(eep, EOF_SYMBOL, &contexts[UNIQUE_CONTEXT_HASH]);
        arienco_done_encoding(eep);
        arienco_delete_encoding_environment(eep);
        encoded_data.resize(code_len / 8);
        out.write(reinterpret_cast<const std::ostream::char_type *>(encoded_data.data()), encoded_data.size());

        return code_len / 8;
    }
};

class PPMADecoder {
public:
    int D = 5;
    std::unordered_map<long long, BiContextType> contexts;

    PPMADecoder() = default;

    explicit PPMADecoder(const int d) {
        D = d;
    }

    int decode(std::vector<unsigned char> &in, std::ofstream &out) {
        int code_len = 0;
        DecodingEnvironmentPtr dep;
        unsigned char *bufptr = in.data();

        dep = arideco_create_decoding_environment();
        arideco_start_decoding(dep, bufptr, 0, &code_len);

        contexts.clear();
        contexts[UNIQUE_CONTEXT_HASH] = BiContextType();
        biari_init_unique_context(&contexts[UNIQUE_CONTEXT_HASH], "#EmptyUniqueContext");
        contexts[DEFAULT_CONTEXT_HASH] = BiContextType();
        biari_init_context(&contexts[DEFAULT_CONTEXT_HASH], "#EmptyDefaultContext");

        std::string currentContext;
        long long i = 0;
        while (true) {
            unsigned int symbol = 0;
            bool symbolDecoded = false;

            std::vector<long long> contextHashes;
            long long currentHash = HashWorker::hashFromString(currentContext);

            for (int depth = std::min(D, (int) currentContext.size()); depth > 0; depth--) {
                contextHashes.push_back(currentHash);
                currentHash = HashWorker::removeFirstSymbolFromHash(currentHash);
            }

            for (long long ctxHash: contextHashes) {
                if (!contexts.contains(ctxHash)) {
                    contexts[ctxHash] = BiContextType();
                    biari_init_context(&contexts[ctxHash], HashWorker::stringFromHash(ctxHash));
                }

                biari_update_context(&contexts[ctxHash], ESC_SYMBOL);
                step_prefix(i);
                symbol = biari_decode_symbol(dep, &contexts[ctxHash]);
                biari_reset_update_context(&contexts[ctxHash], ESC_SYMBOL);
                if (symbol != ESC_SYMBOL) {
                    symbolDecoded = true;
                    break;
                }

            }

            if (!symbolDecoded) {
                biari_update_context(&contexts[DEFAULT_CONTEXT_HASH], ESC_SYMBOL);
                step_prefix(i);
                symbol = biari_decode_symbol(dep, &contexts[DEFAULT_CONTEXT_HASH]);
                if (symbol == ESC_SYMBOL) {
                    step_prefix(i);
                    symbol = biari_decode_symbol(dep, &contexts[UNIQUE_CONTEXT_HASH]);
                    biari_reset_update_context(&contexts[UNIQUE_CONTEXT_HASH], symbol);
                }
                biari_reset_update_context(&contexts[DEFAULT_CONTEXT_HASH], ESC_SYMBOL);
            }
            if (symbol == EOF_SYMBOL) {
                break;
            }

            out.put(symbol);

            long long updateHash = HashWorker::hashFromString(currentContext);
            for (int depth = std::min(D, (int) currentContext.size()); depth > 0; depth--) {
                biari_update_context(&contexts[updateHash], symbol);
                // std::cout << contexts[updateHash].name << std::endl;
                // for (int j = 0; j <= ALPHABET_SIZE; j++) {
                //     // std::cout << j << "\t";
                // }
                // std::cout << std::endl;
                // for (int j = 0; j <= ALPHABET_SIZE; j++) {
                //     // std::cout << contexts[updateHash].cum_freq[j] << "\t";
                // }
                // std::cout << std::endl;
                updateHash = HashWorker::removeFirstSymbolFromHash(updateHash);
            }

            i++;
            biari_update_context(&contexts[DEFAULT_CONTEXT_HASH], symbol);
            currentContext.push_back(symbol);
            if (currentContext.size() > D)
                currentContext = currentContext.substr(currentContext.size() - D);
        }

        arideco_delete_decoding_environment(dep);

        return code_len / 8;
    }
};

int main(int argc, char **argv) {
    auto algorithmMetaInfo = AlgorithmMetaInfo();
    CommandLineParser::parseArgs(algorithmMetaInfo, argc, argv);
    auto inputFileInfo = FileInfo();

    if (FileReader::read(inputFileInfo, algorithmMetaInfo.input_file_name)) {
        return 1;
    }
    //std::cout << inputFileInfo.data.data() << " " << algorithmMetaInfo.isEncoder << std::endl;

    std::ofstream file(algorithmMetaInfo.output_file_name.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "File not found" << std::endl;
        return 1;
    }
    if (algorithmMetaInfo.isEncoder) {
        auto encoder = PPMAEncoder(5);
        const int size = encoder.encode(inputFileInfo.data, file);
        // for (auto i: encoder.contexts) {
        //     // std::cout << i.first << " " << HashWorker::numOfSymbols(i.first) << " " << i.second.name << " " << i.second.
        //             freq_all << " " << i.second.freq_all - i.second.freq[256] << std::endl;
        // }
        std::cout << "BIT SIZE " << size * 8 << " SYMBOL COUNT " << inputFileInfo.data.size() << " DIVISION " << (double)(size * 8) / (double)inputFileInfo.data.size() << std::endl;
    } else {
        auto decoder = PPMADecoder(5);
        decoder.decode(inputFileInfo.data, file);
        // for (auto i: encoder.contexts) {
        //     // std::cout << i.first << " " << HashWorker::numOfSymbols(i.first) << " " << i.second.name << " " << i.second.
        //             freq_all << " " << i.second.freq_all - i.second.freq[256] << std::endl;
        // }
    }
    file.close();

    return 0;
}
