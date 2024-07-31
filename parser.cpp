#include "parser.hpp"
#include <iostream>
#include <cctype>

namespace ccjson
{
    bool validate(std::vector<token> &tokens)
    {
        token previous = start;
        mode arrayFlag = standard;
        for (token tkn : tokens)
        {
            if (tkn == openArray)
            {
                arrayFlag = array;
            }
            else if (tkn == closeArray)
            {
                if (arrayFlag == array)
                {
                    arrayFlag = standard;
                }
                else
                {
                    return false;
                }
            }

            if (tkn == invalid)
            {
                std::cout << "invalid token, automatically invalid\n";
                return false;
            }

            auto iter = transitionMap.find(previous);
            if (iter == transitionMap.end())
            {
                std::cout << "invalid transition from " << previous << '\n';
                return false;
            }

            auto setIter = iter->second.find(tkn);
            if (setIter == iter->second.end())
            {

                std::cout << previous << " does not transition to " << tkn << '\n';
                return false;
            }

            previous = tkn;
        }

        return (arrayFlag == standard);
    }

    std::vector<token> tokenize(std::ifstream &file)
    {
        std::string line;
        std::string::iterator pos;
        std::vector<token> tokens;

        mode arrayFlag = standard;

        // read file line by line
        while (std::getline(file, line))
        {
            // set our position iterator to the line start
            pos = line.begin();

            // we iterate our line without directly using pos++
            while (pos < line.end())
            {

                // essentially read in next character, skip whitespace
                pos = readStandard(line, pos);

                // read while infering how to read, this is to say the iterator will be set in a way such that it lies on the character outside of the token
                // this must be performed differently based on type i/e strings, numbers, keywords vs basic tokens
                std::pair<std::optional<token>, std::string::iterator> readout = readInfer(line, pos, tokens, arrayFlag);
                pos = readout.second;

                // readInfer will return no token if a valid one can't be retrieved
                // we take no token to mean invalid
                tokens.push_back(readout.first.value_or(invalid));

                if (tokens.back() == openArray)
                {
                    arrayFlag = array;
                }
                else if (tokens.back() == closeArray)
                {
                    arrayFlag = standard;
                }
            }
        }

        tokens.push_back(end);
        return tokens;
    }

    namespace
    {
        std::string::iterator readStandard(const std::string &line, const std::string::iterator &position)
        {
            // read and skip whitespace
            std::string::iterator iter;
            for (iter = position; iter < line.end(); iter++)
            {
                if (*iter != ' ')
                    break;
            }

            return iter;
        }

        std::pair<std::string::iterator, bool> readString(const std::string &line, const std::string::iterator &position)
        {
            // read until closing quotation mark
            std::string::iterator iter;
            for (iter = position; iter < line.end(); iter++)
            {
                if (*iter == '"')
                {
                    iter++;
                    return {iter, true};
                }
            }

            return {iter, false};
        }

        std::pair<std::string::iterator, bool> readKeyword(const std::string &line, const std::string::iterator &position)
        {
            // read until no long alphabetic then check validity
            std::string::iterator iter;
            std::string keyword;
            for (iter = position; iter < line.end(); iter++)
            {
                if (!isalpha(*iter))
                {
                    break;
                }
                keyword += *iter;
            }
            auto it = validKeywords.find(keyword);
            bool isValidKeyword = (it != validKeywords.end());
            return {
                iter, isValidKeyword};
        }

        std::string::iterator readNumber(const std::string &line, const std::string::iterator &position)
        {
            // read until no longer numeric
            std::string::iterator iter;
            for (iter = position; iter < line.end(); iter++)
            {
                if (!std::isdigit(*iter))
                {
                    break;
                }
            }

            return iter;
        }

        std::pair<std::optional<token>, std::string::iterator> readInfer(const std::string &line, const std::string::iterator &position, const std::vector<token> &tokens, mode flag)
        {
            char character = *position;
            std::string::iterator outIter = position;

            // try and find character as token
            auto it = tokenMapBasic.find(character);

            if (it != tokenMapBasic.end())
            {
                // if found it is trivial to return the correct token
                outIter++;
                return {it->second, outIter};
            }
            else if (character == '"')
            {
                // keys are strings so we must infer whether the string is a key or a value string
                token last = tokens.back();
                token returnToken;

                // key has a limited set of preceding tokens
                if ((last == comma || last == openObject) && flag != array)
                {
                    returnToken = key;
                }
                else
                {
                    returnToken = string;
                }

                // we must iterate such that our first character is the first element of the string
                outIter++;
                std::pair<std::string::iterator, bool> stringReturn = readString(line, outIter);
                outIter = stringReturn.first;

                if (stringReturn.second)
                {
                    return {returnToken, outIter};
                }
                return {{}, outIter};
            }
            else if (std::isdigit(character))
            {
                // reading a number is trivial
                outIter++;
                outIter = readNumber(line, outIter);
                return {number, outIter};
            }
            else if (isalpha(character))
            {
                // keywords must be specialized to not only read and return iterator but also return validity
                std::pair<std::string::iterator, bool> keywordReturn = readKeyword(line, outIter);
                outIter = keywordReturn.first;
                if (keywordReturn.second)
                {
                    return {keyword, outIter};
                }
                return {{}, outIter};
            }

            // base case probably uneccesary
            outIter++;
            return {{}, outIter};
        }
    }
}