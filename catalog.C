#include "catalog.h"


RelCatalog::RelCatalog(Status &status) :
   HeapFile(RELCATNAME, status)
{
// nothing should be needed here
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
  if (relation.empty())
    return BADCATPARM;

  Status status;
  Record rec;
  RID rid;

  HeapFileScan* heapScan;
  heapScan = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;

  if((status = heapScan->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    delete heapScan;
    return status;
  }

  if (heapScan->scanNext(rid) == OK) {
    if ((status = heapScan->getRecord(rec)) != OK) { 
      delete heapScan;
      return status;
    }
    memcpy(&record, rec.data, rec.length);

    delete heapScan;
    return OK;
  }

  Status status2 = heapScan->endScan();
  if (status == OK) status = status2;
  
  delete heapScan;
  return RELNOTFOUND;
}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;
  Record rec;

  ifs = new InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;

  int length = strlen(record.relName);
  memset(&record.relName[length], 0, sizeof(record.relName - length));
  rec.data = &record;
  rec.length = sizeof(RelDesc);

  status = ifs->insertRecord(rec, rid);
  delete ifs;
  return status;
}

const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;

  hfs = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;

  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    return status;
  }

  if ((status = hfs->scanNext(rid)) != OK) {
      if (status == NORECORDS)
        status = OK;
      return status;
  }
  status = hfs->deleteRecord();
  hfs->endScan();
  return status;
}


RelCatalog::~RelCatalog()
{
// nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
   HeapFile(ATTRCATNAME, status)
{
// nothing should be needed here
}


const Status AttrCatalog::getInfo(const string & relation, 
          const string & attrName,
          AttrDesc &record)
{

  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;

  if (relation.empty() || attrName.empty()) return BADCATPARM;

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    delete hfs;
    return status;
  }

  while (status == OK) {
    if ((status = hfs->scanNext(rid)) != OK){
      return status;
    }
    if((status = hfs->getRecord(rec)) != OK){
      return status;
    }
    memcpy(&record, rec.data, rec.length);
    if (strcmp(record.attrName, attrName.c_str()) == 0){
        return status;
    }
  }
  Status nextStatus = hfs->endScan();
  delete hfs;
  return status;
}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  Record rec;
  InsertFileScan*  ifs;
  Status status;

  ifs = new InsertFileScan(ATTRCATNAME, status);

  int length = strlen(record.relName);
  memset(&record.relName[length], 0, sizeof record.relName - length);
  length = strlen(record.attrName);
  memset(&record.attrName[length], 0, sizeof record.attrName - length);

  rec.data = &record;
  rec.length = sizeof(AttrDesc);
  status = ifs->insertRecord(rec, rid);
  delete ifs;
  return status;
}


const Status AttrCatalog::removeInfo(const string & relation, 
             const string & attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record;
  HeapFileScan*  hfs;

  if (relation.empty() || attrName.empty()) return BADCATPARM;

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    delete hfs;
    return status;
  }

  while(status == OK){
    if((status = hfs->scanNext(rid)) != OK){
      return status;
    }
    if((status = hfs->getRecord(rec)) != OK){
      if(status == NORECORDS){
        status = OK;
      }
      return status;
    }
    memcpy(&record, rec.data, rec.length);

    if(strcmp(record.attrName, attrName.c_str()) == 0){
      status = hfs->deleteRecord();
      hfs->endScan();
      delete hfs;
      return status;
    }
  }
  return status;
}


const Status AttrCatalog::getRelInfo(const string & relation, 
             int &attrCnt,
             AttrDesc *&attrs)
{
  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;
  RelDesc recDesc;
  attrCnt = 0;

  if (relation.empty()) return BADCATPARM;

  hfs = new HeapFileScan(ATTRCATNAME, status);

  if ((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK) {
    delete hfs;
    return status;
  }

  while(status == OK) {
    if ((status = hfs->scanNext(rid)) != OK)
      return status;
    if ((status = hfs->getRecord(rec)) != OK)
      return status;
    attrCnt++;

    memcpy(&attrs[attrCnt - 1], rec.data, rec.length);
  }

  if (status == FILEEOF) {
    if (attrCnt == 0)
      status = RELNOTFOUND;
    else
      status = OK;
  }

  hfs->endScan();
  delete hfs;
  return status;
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

