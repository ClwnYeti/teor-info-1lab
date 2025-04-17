#include <iostream>
#include <vector>
#include <fstream>
#include <string>

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

    void parseArgs(AlgorithmMetaInfo &out, int argc, char **argv) const;
};

struct FileInfo {
    std::string file_name;
    std::vector<unsigned char> data = std::vector<unsigned char>();

    FileInfo() {
    }
};

void CommandLineParser::parseArgs(AlgorithmMetaInfo &out, const int argc, char **argv) const {
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

class PictureReader {
public:
    PictureReader() = default;

    static int read(FileInfo &out, const std::string &input_file_name) {
        std::ifstream file(input_file_name.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "File not found" << std::endl;
            return 1;
        }
        out.file_name = input_file_name;
        file.seekg(0, std::ios::end);
        out.data.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        file.read(reinterpret_cast<std::istream::char_type *>(out.data.data()), out.data.capacity());
        file.close();

        return 0;
    }
};

class PictureWriter {
public:
    PictureWriter() = default;

    static int write(const std::string &output_file_name, const FileInfo &in) {
        std::ofstream file(output_file_name.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "File not found" << std::endl;
            return 1;
        }

        file.write(reinterpret_cast<const std::ostream::char_type *>(in.data.data()), in.data.size());
        file.close();

        return 0;
    }
};

int main()
{
}
