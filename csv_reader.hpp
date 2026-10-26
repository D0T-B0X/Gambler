#ifndef CSV_READER_H
#define CSV_READER_H

#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>

class CSVReader {
public:
    static void read_csv(std::string path, std::vector<uint32_t>& arr) {
        std::ifstream file(path);

        if (!file.is_open()) {
            std::cerr << "ERROR::Could not open file" << std::endl; 
            exit(-1);
        }
        
        std::string line;
        std::getline(file, line);
        uint32_t size = std::stoul(line);
        while(std::getline(file, line))
            arr.emplace_back(std::stoull(line));
    }
};

#endif