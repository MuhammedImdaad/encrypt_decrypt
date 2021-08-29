#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <bitset>

using namespace std;

int dictionary_creation(string name, vector<string> &codelines, map<string, string> &dictionary, vector<string> &dictIter);
void compress(string name, vector<string> &codelines, vector<string> &dictIter);
void compression(string inputFile, string outputFile);
string formatting(int &linePtr, string line);

string original_binary_000(string);
string run_length_001(int);
string bitmask_010(string line, vector<string> &dictIter);
string mismatch_1bit_011(string line, vector<string> &dictIter);
string mismatch_2bit_100(string line, vector<string> &dictIter);
string mismatch_4bit_101(string line, vector<string> &dictIter);
string mismatch_2bit_any_110(string line, vector<string> &dictIter);
string direct_match_111(string line);

int extract_dict(string name, map<string, string> &dictionary);
void decompress(string, string);
string read_file(ifstream &file, int length);
void decompression(string inputFile, string outputFile);

string d_original_binary_000(ifstream &file);
void d_run_length_001(string previous, map<string, int> &code_switch, ifstream &file, ofstream &outFile);
string d_bitmask_010(ifstream &file);
string d_mismatch_1bit_011(ifstream &file);
string d_mismatch_2bit_100(ifstream &file);
string d_mismatch_4bit_101(ifstream &file);
string d_mismatch_2bit_any_110(ifstream &file);
string d_direct_match_111(string);

map<string, string> dictionary;

int main(int argc, char *argv[])
{
    if (string(argv[1]) == "1")
        compression("original.txt", "cout.txt");
    else if (string(argv[1]) == "2")
        decompression("compressed.txt", "dout.txt");
    return 0;
}

void compression(string inputFile, string outputFile)
{
    vector<string> codelines;
    vector<string> dictIter;

    if (dictionary_creation(inputFile, codelines, dictionary, dictIter))
    {
        compress(outputFile, codelines, dictIter);
    }
}

string formatting(int &linePtr, string line)
{
    int formatSize = 32;
    int size = line.size();

    if (size > formatSize)
    {
        string out = line.substr(0, formatSize - linePtr) + "\n" + line.substr(formatSize - linePtr, formatSize) + "\n";
        linePtr += size;
        linePtr %= formatSize;
        out += line.substr(size - linePtr);
        return out;
    }
    else
    {
        linePtr += size;
        if (linePtr > formatSize)
        {
            linePtr %= formatSize;
            return line.substr(0, size - linePtr) + "\n" + line.substr(size - linePtr);
        }
        else
            return line;
    }
}

void compress(string name, vector<string> &codelines, vector<string> &dictIter)
{
    ofstream outFile;
    string result;
    string preItr = "";
    int sameCount = 0;
    int formatCount = 0;
    outFile.open(name);
    if (outFile.is_open())
    {
        for (auto &itr : codelines)
        {
            if (preItr == itr && sameCount < 8)
            {
                sameCount++;
            }

            else
            {
                if (sameCount == 8)
                {
                    result = formatting(formatCount, run_length_001(sameCount - 1));
                    outFile << result;
                    sameCount = 0;
                }
                else if (preItr != itr && sameCount > 0)
                {
                    result = formatting(formatCount, run_length_001(sameCount - 1));
                    outFile << result;
                    sameCount = 0;
                }

                result = formatting(formatCount, direct_match_111(itr));
                if (result == "")
                {
                    result = formatting(formatCount, mismatch_1bit_011(itr, dictIter));
                    if (result == "")
                    {
                        result = formatting(formatCount, mismatch_2bit_100(itr, dictIter));
                        if (result == "")
                        {
                            result = formatting(formatCount, mismatch_4bit_101(itr, dictIter));
                            if (result == "")
                            {
                                result = formatting(formatCount, bitmask_010(itr, dictIter));
                                if (result == "")
                                {
                                    result = formatting(formatCount, mismatch_2bit_any_110(itr, dictIter));
                                    if (result == "")
                                    {
                                        result = formatting(formatCount, original_binary_000(itr));
                                        outFile << result;
                                    }
                                    else
                                        outFile << result;
                                }
                                else
                                    outFile << result;
                            }
                            else
                                outFile << result;
                        }
                        else
                        {
                            outFile << result;
                        }
                    }
                    else
                        outFile << result;
                }
                else
                    outFile << result;
            }
            preItr = itr;
        }
        string s(32 - formatCount, '0');
        outFile << s << "\n"
                << "xxxx" << endl;
        for (auto &it : dictIter)
            outFile << it << endl;
        outFile.close();
    }
}

string bitmask_010(string line, vector<string> &dictIter)
{
    for (auto &itr : dictIter)
    {
        string result = (bitset<32>(line) ^ bitset<32>(itr)).to_string();
        int first = result.find("1");
        if (first != string::npos)
        {
            int second = result.find("1", first + 4);
            if (second == string::npos)
            {
                string out = "010" + bitset<5>(first).to_string() + result.substr(first, 4) + dictionary[itr];
                // cout << out << endl;
                return out;
            }
        }
    }

    return "";
}

string mismatch_2bit_any_110(string line, vector<string> &dictIter)
{
    for (auto &itr : dictIter)
    {
        string result = (bitset<32>(line) ^ bitset<32>(itr)).to_string();
        int first = result.find("1");
        if (first != string::npos)
        {
            int second = result.find("1", first + 1);
            if (second != string::npos)
            {
                if (result.find("1", second + 1) == string::npos)
                {
                    string out = "110" + bitset<5>(first).to_string() + bitset<5>(second).to_string() + dictionary[itr];
                    // cout << out << endl;
                    return out;
                }
            }
        }
    }
    return "";
}

string mismatch_4bit_101(string line, vector<string> &dictIter)
{
    for (auto &itr : dictIter)
    {
        string result = (bitset<32>(line) ^ bitset<32>(itr)).to_string();
        int zero = result.find("1");
        int first = result.find("1111");
        if ((zero == first) && (first != string::npos))
        {
            int second = result.find("1", first + 4);
            if (second == string::npos)
            {
                string out = "101" + bitset<5>(first).to_string() + dictionary[itr];
                // cout << out << endl;
                return out;
            }
        }
    }

    return "";
}

string mismatch_2bit_100(string line, vector<string> &dictIter)
{
    for (auto &itr : dictIter)
    {
        string result = (bitset<32>(line) ^ bitset<32>(itr)).to_string();
        // cout << result << endl;
        int zero = result.find("1");
        int first = result.find("11");
        if ((zero == first) && (first != string::npos))
        {
            int second = result.find("1", first + 2);
            if (second == string::npos)
            {
                string out = "100" + bitset<5>(first).to_string() + dictionary[itr];
                // cout << out << endl;
                return out;
            }
        }
    }
    return "";
}

string mismatch_1bit_011(string line, vector<string> &dictIter)
{
    for (auto &itr : dictIter)
    {
        string result = (bitset<32>(line) ^ bitset<32>(itr)).to_string();
        int first = result.find("1");
        if (first != string::npos)
        {
            int second = result.find("1", first + 1);
            if (second == string::npos)
            {
                string out = "011" + bitset<5>(first).to_string() + dictionary[itr];
                // cout << out << endl;
                return out;
            }
        }
    }
    return "";
}

string direct_match_111(string line)
{
    auto itr = dictionary.find(line);
    if (itr != dictionary.end())
    {
        string out = "111" + itr->second;
        // cout << out << endl;
        return out;
    }

    return "";
}

string run_length_001(int repretion)
{
    string out = "001" + bitset<3>(repretion).to_string();
    // cout << out << endl;
    return out;
}

string original_binary_000(string line)
{
    return "000" + line;
}

int dictionary_creation(string name, vector<string> &codelines, map<string, string> &dictionary, vector<string> &dictIter)
{
    ifstream file;
    string line;
    int size;
    map<string, int> frequencyTable;
    string keys[16] = {
        "0000", "0001", "0010", "0011",
        "0100", "0101", "0110", "0111",
        "1000", "1001", "1010", "1011",
        "1100", "1101", "1110", "1111"};

    file.open(name);
    if (file.is_open())
    {
        while (!file.eof())
        {
            getline(file, line);
            if (file.good())
            {
                codelines.push_back(line);
                frequencyTable[line]++;
            }
        }
        file.close();
        size = codelines.size();
        const int SIZE_DICT = 16;
        for (int i = 0; i < SIZE_DICT; i++)
        {
            if (frequencyTable.empty())
                break;
            int max_value = 0;
            string max_word = "";
            for (const auto &kv : codelines)
            {
                if (frequencyTable[kv] > max_value)
                {
                    max_value = frequencyTable[kv];
                    max_word = kv;
                }
            }
            frequencyTable.erase(max_word);
            dictionary[max_word] = keys[i];
            dictIter.push_back(max_word);
        }
        return 1;
    }
    else
        // cout << "Uncompressed file not found" << endl;
        return 0;
}

void decompression(string inputFile, string outputFile)
{
    if (extract_dict(inputFile, dictionary))
    {
        decompress(inputFile, outputFile);
    }
}

string read_file(ifstream &file, int length)
{
    string txt;
    char c;
    int i = 0;
    if (file.is_open())
    {
        for (i; i < length; i++)
        {
            if (file.get(c) && file.good())
            {
                if (c == '\n')
                    file.get(c);
                else if (c == 'x')
                    return "";
                txt.push_back(c);
            }
        }
    }
    return txt;
}

void decompress(string inputFileName, string outFileName)
{
    map<string, int> code_switch{{"000", 0}, {"001", 1}, {"010", 2}, {"011", 3}, {"100", 4}, {"101", 5}, {"110", 6}, {"111", 7}};
    ifstream file;
    ofstream outFile;
    string decompressed;
    file.open(inputFileName);
    outFile.open(outFileName);
    bool go = 1;
    if (outFile.is_open())
    {
        while (go)
        {
            int code = code_switch[read_file(file, 3)];
            switch (code)
            {
            case 0:
                decompressed = d_original_binary_000(file);
                if (decompressed == "")
                    go = 0;
                else
                {
                    outFile << decompressed << endl;
                    // cout << decompressed << endl;
                }
                break;
            case 1:
                d_run_length_001(decompressed, code_switch, file, outFile);
                break;
            case 2:
                decompressed = d_bitmask_010(file);
                outFile << decompressed << endl;
                // cout << decompressed << endl;
                break;
            case 3:
                decompressed = d_mismatch_1bit_011(file);
                outFile << decompressed << endl;
                // cout << decompressed << endl;
                break;
            case 4:
                decompressed = d_mismatch_2bit_100(file);
                outFile << decompressed << endl;
                // cout << decompressed << endl;
                break;
            case 5:
                decompressed = d_mismatch_4bit_101(file);
                outFile << decompressed << endl;
                // cout << decompressed << endl;
                break;
            case 6:
                decompressed = d_mismatch_2bit_any_110(file);
                outFile << decompressed << endl;
                // cout << decompressed << endl;
                break;
            case 7:
                decompressed = d_direct_match_111(read_file(file, 4));
                outFile << decompressed << endl;
                // cout << decompressed << endl;
                break;
            default:
                break;
            }
        }
        file.close();
        outFile.close();
    }
}

string d_mismatch_1bit_011(ifstream &file)
{
    int location = bitset<5>(read_file(file, 5)).to_ulong();
    string code = dictionary[read_file(file, 4)];
    string out = code.substr(0, location);
    if (code[location++] == '0')
        out += "1";
    else
        out += "0";
    out += code.substr(location);
    return out;
}

string d_mismatch_4bit_101(ifstream &file)
{
    int location = bitset<5>(read_file(file, 5)).to_ulong();
    string code = dictionary[read_file(file, 4)];
    string out = code.substr(0, location);
    for (int i = 0; i < 4; i++)
    {
        if (code[location++] == '0')
            out += "1";
        else
            out += "0";
    }
    out += code.substr(location);
    return out;
}

string d_mismatch_2bit_100(ifstream &file)
{
    int location = bitset<5>(read_file(file, 5)).to_ulong();
    string code = dictionary[read_file(file, 4)];
    string out = code.substr(0, location);
    for (int i = 0; i < 2; i++)
    {
        if (code[location++] == '0')
            out += "1";
        else
            out += "0";
    }
    out += code.substr(location);
    return out;
}

string d_mismatch_2bit_any_110(ifstream &file)
{
    int location1 = bitset<5>(read_file(file, 5)).to_ulong();
    int location2 = bitset<5>(read_file(file, 5)).to_ulong();
    string code = dictionary[read_file(file, 4)];
    string out = code.substr(0, location1);

    if (code[location1++] == '0')
        out += "1";
    else
        out += "0";
    out += code.substr(location1, location2 - location1);
    if (code[location2++] == '0')
        out += "1";
    else
        out += "0";
    out += code.substr(location2);
    return out;
}

string d_bitmask_010(ifstream &file)
{
    int location = bitset<5>(read_file(file, 5)).to_ulong();
    int l = location;
    string bitmask = read_file(file, 4);
    string code = dictionary[read_file(file, 4)];
    string xored = "";
    for (int i = 0; i < 4; i++)
    {
        if (code[l++] == bitmask[i])
            xored += "0";
        else
            xored += "1";
    }
    return code.substr(0, location) + xored + code.substr(l);
}

void d_run_length_001(string previous, map<string, int> &code_switch, ifstream &file, ofstream &outFile)
{
    int count = code_switch[read_file(file, 3)] + 1;
    for (int i = 0; i < count; i++)
    {
        outFile << previous << endl;
        // cout << previous << endl;
    }
}

string d_original_binary_000(ifstream &file)
{
    string orig = read_file(file, 32);
    if (orig.size() != 32)
        return "";
    return orig;
}

string d_direct_match_111(string key)
{
    return dictionary[key];
}

int extract_dict(string name, map<string, string> &dictionary)
{
    ifstream file;
    string line;
    string breakpoint = "xxxx";
    int dict_size = 16;
    string keys[dict_size] = {
        "0000", "0001", "0010", "0011",
        "0100", "0101", "0110", "0111",
        "1000", "1001", "1010", "1011",
        "1100", "1101", "1110", "1111"};

    file.open(name);
    if (file.is_open())
    {
        while (!file.eof())
        {
            getline(file, line);
            if (file.good())
            {
                if (line == breakpoint)
                {
                    for (int i = 0; i < dict_size; i++)
                    {
                        getline(file, line);
                        dictionary[keys[i]] = line;
                    }
                    file.close();
                    return 1;
                }
            }
        }
    }
    else
    {
        // cout << "compressed file not found" << endl;
    }
    return 0;
}
