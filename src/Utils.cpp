#include "Utils.h"

#include <sstream>
#include <algorithm>
#include <libxslt/xsltutils.h>

namespace Utils
{

std::string GetLastXmlError()
{
    std::string result;

    xmlError *err = xmlGetLastError();
    if (!err)
        return result;

    std::stringstream stream;

    if (err->message)
        stream << err->message;

    if (err->file)
        stream << "file: " << err->file << '\n';

    if (err->line)
        stream << "line: " << err->line << '\n';

    // Blame libxml for calling the column field 'int2', not me...
    if (err->int2)
        stream << "column: " << err->int2 << '\n';

    result = stream.str();

    TrimString(result);

    return result;
}

void TrimString(std::string &string)
{
    // Trim the left side of the string by only keeping the characters that are not whitespaces
    string.erase(string.begin(), std::find_if(string.begin(), string.end(), [](unsigned char ch) { return !std::isspace(ch); }));

    // Trim the right side of the string by only keeping the characters that are not whitespaces
    string.erase(std::find_if(string.rbegin(), string.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), string.end());
}

}
