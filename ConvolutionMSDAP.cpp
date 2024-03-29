#pragma region Headers

// Include all required header files here
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <bitset>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdint.h>
#include <fstream>
#include <vector>

#pragma endregion

#pragma region Global Variables

// Include Vector variables here
std::vector<uint16_t> hCoeff; // To store data from coeff.in
std::vector<uint16_t> xData;  // To store data from data.in
std::vector<uint16_t> rJData;  // To store data from Rj.in
// End Vector variables

#pragma endregion

#pragma region Parse Algorithm

/// @brief The following funtion is used to parse the docuemnt and read the hex numbers in the given file path and store the data in the desired globally initilized vectors
/// @param filePath variable type = string
/// @param selectVectorToStoreData variable type = uint8_t
/// @return 0 - if no error else 1
int parse(std::string filePath, uint8_t selectVectorToStoreData)
{
    std::ifstream inputFile(filePath); // Input file path

    if (inputFile.is_open()) // If file open
    {
        std::string line;                     // string to store data to process
        while (std::getline(inputFile, line)) // Read each line till the end of file
        {
            //std::istringstream iss(line); // To convert string to hex number
            std::string hexString = line.substr(0,4);
            //iss >> hexString;

            switch (selectVectorToStoreData)
            {
            case 1: // Store data to hCoeff
            {
                uint16_t hexCoeff;
                std::stringstream(hexString) >> std::hex >> hexCoeff; // Convert to hex
                hCoeff.push_back(hexCoeff);                           // Add data at the end of the vector
            }
            break;
            case 2: // Store data to xData
            {
                uint16_t hexData;
                std::stringstream(hexString) >> std::hex >> hexData; // Convert to hex
                xData.push_back(hexData);                            // Add data at the end of the vector
            }
            break;
            case 3:
            {
                uint16_t hexRj;
                std::stringstream(hexString) >> std::hex >> hexRj; // Convert to hex
                rJData.push_back(hexRj);
            }
            break;

            default: // Do nothing
                break;
            }
        }
        inputFile.close(); // Close file
    }
    else // If file not open
    {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }
    return 0;
}

#pragma endregion

#pragma region Calculate x(n-coeff) value

/// @brief The following function is used to calculate the x(n-coeff) value
/// @param nValue index of data.in variable type = uint16_t
/// @param kValue value of coeff.in variable type = uint16_t
/// @param flag varaible type = boolean used to indicate sign bit
/// @return value of x(n-coeff) is returned
int64_t calculateXValue(uint16_t nValue, uint16_t kValue, bool flag)
{
    int64_t xValue = 0;
    if (nValue - kValue >= 0)   //x(-255) = ...... = x(-1) = 0
    {
        xValue = xData[nValue - kValue] << 16;
        if (flag == true)   //If sign bit high
        {
            xValue = ~xValue + 1;
        }
    }
    return xValue;
}

#pragma endregion

#pragma region Convolution Function

/// @brief The function is used to perform convolution for each data set in data.in file and push the result in yOutput vector
/// @param filePath Variable type = string
void convolutionFunction(std::string filePath)
{
    int64_t result = 0;
    uint16_t i, j, k, readSignBit;
    bool signFlag = false;
    uint16_t tmp, count = 0;
    std::vector<int64_t> yOutput(xData.size(), 0); // To store output data computed from the above two vectors

    for (k = 0; k < xData.size(); k++)  //To calculate y(k) output
    {
        j = 0;
        for (i = 0; i < rJData.size(); i++)
        {
            while (count != rJData[i])
            {
                readSignBit = 0x0100 & hCoeff[j];
                if (readSignBit == 0x0100)
                {
                    signFlag = true;
                }
                else
                {
                    signFlag = false;
                }
                tmp = hCoeff[j] & 0x00FF;
                result = result + calculateXValue(k, tmp, signFlag);
                count = count + 1;
                j = j + 1;
            }
            result = result >> 1;
            count = 0;
        }
        yOutput[k] = result;
        result = 0;
    }

    std::ofstream file(filePath);   //Write 10bit output value to file
    for (auto value : yOutput)
    {
        value = value & 0x000000ffffffffff;
        file << std::hex << std::setw(10) << std::setfill('0') << std::uppercase << value << std::endl; // Write value to file
    }
}

#pragma endregion

#pragma region Main Function

/// @brief Main Function to accept three files namely coeff, data and output. Compute the convolution from coeff and data and store it in output
/// @param argc
/// @param argv
/// @return 0 - if no error else 1
int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << "<path/to/coeff.in> <path/to/rj.in> <path/to/data.in> <path/to/output.out>\n";
        return 1;
    }

    int fileParseStatus; // Variable to store the parsing status

    fileParseStatus = parse(argv[1], 1); // Parse coeff.in file

    if (fileParseStatus == 1) // If failed to parse file end program
    {
        std::cout << "Failed to read Coeff.in file" << std::endl;
        return 1;
    }

    fileParseStatus = parse(argv[3], 2); // Parse data.in file

    if (fileParseStatus == 1) // If failed to parse the file end program
    {
        std::cout << "Failed to read data.in file" << std::endl;
        return 1;
    }

    fileParseStatus = parse(argv[2], 3); // Parse Rj.in file

    if (fileParseStatus == 1) // If failed to parse the file end program
    {
        std::cout << "Failed to read Rj.in file" << std::endl;
        return 1;
    }

    convolutionFunction(argv[4]);

    std::cout << "Convolution result uploaded to file: " << argv[4] << std::endl;

    return 0;
}

#pragma endregion