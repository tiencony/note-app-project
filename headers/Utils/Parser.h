#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <string>
#include <vector>

std::set<std::string> extractTags(const std::string& content);
std::string normalizeTag(const std::string& tag);
std::set<std::string> extractLinks(const std::string& content);
std::string normalizeTitle(const std::string& s);
std::string replaceLink(const std::string& content, const std::string& oldTitle, const std::string& newTitle);


struct LinkSpan{
    std::size_t innerBegin; // Vi tri ngay sau "[["
    std::size_t innerEnd;   // Vi tri truoc "]]"
};

struct LinkParts{
    std::string target; // Da cat dau "|, #"
    std::string suffix; // Sau dau "|, #"
};

LinkParts parseLink(const std::string& inner);
std::vector<LinkSpan> findLinkSpans(const std::string& content);

#endif