#pragma once

#ifndef PARSER
#define PARSER

#include <unordered_map> //provide O(1) lookups for tokens
#include <unordered_set>
#include <string>
#include <fstream>
#include <vector> //store tokenlist

#include <utility>  //pairs
#include <optional> //allows flexibility

namespace ccjson
{

    enum token
    {
        openObject,
        closeObject,
        key,
        colon,
        comma,
        invalid,
        string,
        number,
        keyword,
        openArray,
        closeArray,
        start,
        end,
    };

    namespace
    {
        enum mode
        {
            standard,
            array
        };

        // map basic characters to tokens
        const std::unordered_map<char, token> tokenMapBasic =
            {
                {'{', openObject},
                {'}', closeObject},
                {'[', openArray},
                {']', closeArray},
                {':', colon},
                {',', comma},
        };

        // validate keywords
        const std::unordered_set<std::string> validKeywords =
            {
                "true",
                "false",
                "null"};

        const std::unordered_map<token, std::unordered_set<token>> transitionMap =
            {
                {start, {openObject}},
                {openObject, {key, closeObject}},
                {key, {colon}},
                {colon, {string, openArray, openObject, keyword, number}},
                {string, {closeArray, closeObject, comma}},
                {openArray, {closeArray, keyword, number, string}},
                {keyword, {closeArray, closeObject, comma}},
                {number, {closeArray, closeObject, comma}},
                {comma, {key, string}},
                {closeObject, {end, comma, closeObject}},
                {closeArray, {comma, closeObject}}};

    }

    namespace
    {
        std::string::iterator readStandard(const std::string &line, const std::string::iterator &position);

        std::pair<std::string::iterator, bool> readString(const std::string &line, const std::string::iterator &position);

        std::pair<std::string::iterator, bool> readKeyword(const std::string &line, const std::string::iterator &position);

        std::string::iterator readNumber(const std::string &line, const std::string::iterator &position);

        std::pair<std::optional<token>, std::string::iterator> readInfer(const std::string &line, const std::string::iterator &position, const std::vector<token> &tokens, mode flag);
    }

    std::vector<token> tokenize(std::ifstream &file);

    bool validate(std::vector<token> &tokens);


}
#endif