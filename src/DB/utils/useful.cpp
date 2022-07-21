#include "useful.h"

using namespace std;

vector<string> split_by_space(string line) {
    vector<string> tokens;
    size_t prev = line.find_first_not_of(" ");
    size_t pos;

    while ((pos = line.find_first_of(" ", prev)) != string::npos)
    {
        if (pos > prev)
            tokens.push_back(line.substr(prev, pos - prev));
        prev = pos + 1;
    }
    if (prev < line.length())
        tokens.push_back(line.substr(prev, string::npos));

    return tokens;
}

vector<string> split(string input, char delimiter) {
    vector<string> answer;
    stringstream ss(input);
    string temp;

    while (getline(ss, temp, delimiter)) {
        answer.push_back(temp);
    }

    return answer;
}