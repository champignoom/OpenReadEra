#ifndef EPUBFMT_H
#define EPUBFMT_H

#include "lvtinydom.h"

class EncryptedItem;

class EncryptedItemCallback
{
public:
    virtual void addEncryptedItem(EncryptedItem* item) = 0;
    virtual ~EncryptedItemCallback() {}
};

class EncryptedDataContainer : public LVContainer, public EncryptedItemCallback
{
    LVContainerRef _container;
    LVPtrVector<EncryptedItem> _list;
    LVArray<lUInt8> _fontManglingKey;
public:
    EncryptedDataContainer(LVContainerRef baseContainer);
    virtual LVContainer* GetParentContainer();
    //virtual const LVContainerItemInfo* GetObjectInfo(const wchar_t * pname);
    virtual const LVContainerItemInfo* GetObjectInfo(int index);
    virtual int GetObjectCount() const;
    /// returns object size (file size or directory entry count)
    virtual lverror_t GetSize(lvsize_t* pSize);
    virtual LVStreamRef OpenStream(const lChar16* fname, lvopen_mode_t mode);
    virtual LVStreamRef OpenStreamByPackedSize(uint32_t size);
    /// returns stream/container name, may be NULL if unknown
    virtual const lChar16* GetName();
    /// sets stream/container name, may be not implemented for some objects
    virtual void SetName(const lChar16* name);
    virtual void addEncryptedItem(EncryptedItem* item);
    EncryptedItem* findEncryptedItem(const lChar16* name);
    bool isEncryptedItem(const lChar16* name);
    bool setManglingKey(lString16 key);
    bool hasUnsupportedEncryption();
    bool open();
    static void createEncryptedEpubWarningDocument(CrDom *m_doc);
};

bool DetectEpubFormat(LVStreamRef stream);
bool ImportEpubDocument(LVStreamRef stream, CrDom * doc,  bool firstpage_thumb);
lString16 EpubGetRootFilePath(LVContainerRef m_arc);

void GetEpubMetadata(CrDom *dom,
        lString16 *res_title,
        lString16 *res_authors,
        lString16 *res_lang,
        lString16 *res_series,
        int *res_series_number,
        lString16 *res_genre,
        lString16 *res_annotation,
        int *fontcount);

bool checkEpubJapaneseVertical(CrDom *dom, LVContainerRef container, lString16 code_base);

LVStreamRef GetEpubCoverImage(CrDom *dom, LVContainerRef container,lString16 code_base );

bool ImportDocxDocument(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb);
bool ImportOdtDocument(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb);
//lString16 DocxGetMainFilePath(LVContainerRef m_arc);

#endif // EPUBFMT_H
