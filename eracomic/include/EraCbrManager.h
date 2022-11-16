/*
 * Copyright (C) 2013-2021 READERA LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Developers: ReadEra Team (2013-2021), Playful Curiosity (2013-2021),
 * Tarasus (2018-2021).
 */
//
// Created by Tarasus on 22.12.2020.
//

#ifndef CODE_READERA_TARASUS_ERACBRMANAGER_H
#define CODE_READERA_TARASUS_ERACBRMANAGER_H


#include <map>
#include "../dmc_unrar/dmc_unrar.h"
#include "EraComicManager.h"

std::string stripPath(std::string filename);

std::string get_file_comment(dmc_unrar_archive *archive, dmc_unrar_size_t i);

std::string get_filename(dmc_unrar_archive *archive, dmc_unrar_size_t i);


class CbrManager: public ComicManager
{
public:
    dmc_unrar_archive arc;

    void openDocument(std::string path, int fd);
    void closeDocument();
    ComicFile * loadFile(uint32_t index);
    bool getPageInfo(uint32_t index, int *w, int *h);
};




#endif //CODE_READERA_TARASUS_ERACBRMANAGER_H
