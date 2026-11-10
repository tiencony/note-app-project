#ifndef PARSER_H
#define PARSER_H

#include <set>
#include <string>

std::set<std::string> extractTags(const std::string& content);
std::string normalizeTag(const std::string& tag);
std::set<std::string> extractLinks(const std::string& content);
std::string normalizeTitle(const std::string& s);
std::string replaceLink(const std::string& content, const std::string& oldTitle, const std::string& newTitle);


#endif