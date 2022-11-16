//
// Created by Tarasus on 1/2/2021.
//

#ifndef CODE_READERA_TARASUS_ERACOMICRARUTILS_H
#define CODE_READERA_TARASUS_ERACOMICRARUTILS_H

#include <vector>
#include <string>

std::vector<std::string> getRarEntries(std::string path, int fd);

bool extractRarEntry(std::string arc_path, int fd, int entrynum, std::string path_out);

#endif //CODE_READERA_TARASUS_ERACOMICRARUTILS_H
