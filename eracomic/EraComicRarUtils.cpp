//
// Created by Tarasus on 1/2/2021.
//

#include <EraCbrManager.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "EraComicRarUtils.h"

#include "../orebridge/include/ore_log.h"
#include "../dmc_unrar/dmc_unrar.h"
#include <errno.h>


std::vector<std::string> getRarEntries(std::string path, int fd)
{
    std::vector<std::string> res;

    dmc_unrar_archive arc;

    dmc_unrar_return err = dmc_unrar_archive_init(&arc);
    if (err != DMC_UNRAR_OK)
    {
        LE("getRarEntries: Archive init failed: %s", dmc_unrar_strerror(err));
        return res;
    }

    if(fd>0)
    {
        //LE("CbrManager openDocument read via fd (%d)",fd);
        struct stat stat;
        if (fstat(fd, &stat) == -1)
        {
            LE("getRarEntries: FAILED TO OPEN RAR [%s] : failed fstat %d : %s",path.c_str(), errno, strerror(errno));
            return res;
        }

        size_t size = (int) stat.st_size;
        auto mem = (unsigned  char*)mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
        if ( mem == MAP_FAILED )
        {
            LE("getRarEntries: FAILED TO OPEN RAR [%s] : mmap failed %d : %s",path.c_str(), errno, strerror(errno));
            return res;
        }
        err = dmc_unrar_archive_open_mem(&arc, mem, size);
    }
    else
    {
        err = dmc_unrar_archive_open_path(&arc, path.c_str());
    }

    if (err != DMC_UNRAR_OK)
    {
        LE("getRarEntries: FAILED TO OPEN RAR [%s] : %s",path.c_str(),dmc_unrar_strerror(err));
        return res;
    }

    int dmc_filecount = dmc_unrar_get_file_count(&arc);

    for (int i = 0; i < dmc_filecount; i++)
    {
        std::string entrystr;
        std::string name = get_filename(&arc, i);
        const dmc_unrar_file* fstat = dmc_unrar_get_file_stat(&arc,i);

        std::replace( name.begin(), name.end(), '|', '?');

        entrystr += std::to_string(i) + "|";
        entrystr += name + "|";
        entrystr += std::to_string(fstat->uncompressed_size) + "|";
        entrystr += std::to_string(fstat->compressed_size);

        //LE("entry = [%s]",entrystr.c_str());
        res.emplace_back(entrystr);
    }
    dmc_unrar_archive_close(&arc);
    return res;
}


bool extractRarEntry(std::string arc_path, int fd, int entrynum, std::string path_out)
{
    std::vector<std::string> res;

    dmc_unrar_archive arc;

    dmc_unrar_return err = dmc_unrar_archive_init(&arc);
    if (err != DMC_UNRAR_OK)
    {
        LE("extractRarEntry: Archive init failed: %s", dmc_unrar_strerror(err));
        return false;
    }

    if(fd>0)
    {
        //LE("CbrManager openDocument read via fd (%d)",fd);
        struct stat stat;
        if (fstat(fd, &stat) == -1)
        {
            LE("extractRarEntry: FAILED TO OPEN RAR [%s] : failed fstat %d : %s",arc_path.c_str(), errno, strerror(errno));
            return false;
        }

        size_t size = (int) stat.st_size;
        auto mem = (unsigned  char*)mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
        if ( mem == MAP_FAILED )
        {
            LE("extractRarEntry: FAILED TO OPEN RAR [%s] : mmap failed %d : %s",arc_path.c_str(), errno, strerror(errno));
            return false;
        }
        err = dmc_unrar_archive_open_mem(&arc, mem, size);
    }
    else
    {
        err = dmc_unrar_archive_open_path(&arc, arc_path.c_str());
    }

    if (err != DMC_UNRAR_OK)
    {
        LE("extractRarEntry: FAILED TO OPEN RAR [%s] : %s", arc_path.c_str(), dmc_unrar_strerror(err));
        return false;
    }

    int dmc_filecount = dmc_unrar_get_file_count(&arc);
    if (entrynum > dmc_filecount)
    {
        LE("extractRarEntry: no entry %d exists in arc [%s] (max = %d)", entrynum, arc_path.c_str(), dmc_filecount);
        dmc_unrar_archive_close(&arc);
        return false;
    }


    std::string name = get_filename(&arc, entrynum);

    dmc_unrar_return extracted = dmc_unrar_extract_file_to_path(&arc, entrynum, path_out.c_str(), NULL, false);
    if (extracted != DMC_UNRAR_OK)
    {
        LE("extractRarEntry: failed extracting entry %d from [%s] : [%s]",entrynum,arc_path.c_str(),dmc_unrar_strerror(extracted));
        dmc_unrar_archive_close(&arc);
        return false;
    }

    dmc_unrar_archive_close(&arc);
    return true;
}
