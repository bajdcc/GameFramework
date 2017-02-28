#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>

std::string base64_encode(const std::vector<byte>& data);
std::vector<byte> base64_decode(const std::string& data);

#endif