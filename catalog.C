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

  HeapFileScan *heapScan = new HeapFileScan(RELCATNAME, status);

  if((status = heapScan->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    return status;
  }
  if((status = heapScan->scanNext(rid)) != OK){
    return status;
  }
  if((status = heapScan->getRecord(rec)) != OK){
    return status;
  }

  memcpy((void *)&record, rec.data, rec.length);

  return status;
}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;
  Record rec;

  ifs = new InsertFileScan(RELCATNAME, status);

  rec.data = &record;
  rec.length = sizeof(RelDesc);

  status = ifs->insertRecord(rec, rid);
  return status;
}

const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;

  hfs = new HeapFileScan(RELCATNAME, status);

  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    return status;
  }
  if((status = hfs->scanNext(rid)) != OK){
      if(status == NORECORDS){
        status = OK;
      }
    return status;
  }

  status = hfs->deleteRecord();
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
    return status;
  }

  while(status == OK){
    if((status = hfs->scanNext(rid)) != OK){
      return status;
    }
    if((status = hfs->getRecord(rec)) != OK){
      return status;
    }
    memcpy((void*)&record, rec.data, rec.length);
    if(strcmp(record.attrName, attrName.c_str()) == 0){
        return status;
    }
  }

  return status;
}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  Record rec;
  InsertFileScan*  ifs;
  Status status;

  ifs = new InsertFileScan(ATTRCATNAME, status);
  rec.data = &record;
  rec.length = sizeof(AttrDesc);
  status = ifs->insertRecord(rec, rid);
  
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
    if(strcmp(record.attrName, attrName.c_str()) == 0){
      status = hfs->deleteRecord();
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
  int i;

  if (relation.empty()) return BADCATPARM;

  if((status = relCat->getInfo(relation, recDesc)) != OK){
    return status;
  }

  attrCnt = recDesc.attrCnt;
  attrs = new AttrDesc[attrCnt];

  //attrs = (AttrDesc*) malloc(attrCnt * sizeof(AttrDesc));

  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    return status;
  }

  for(i = 0; i < attrCnt; i++){
    if((status = hfs->scanNext(rid)) != OK){
      return status;
    }
    if((status = hfs->getRecord(rec)) != OK){
      return status;
    }
    memcpy(&attrs[i], rec.data, rec.length);
  }
  if(status == FILEEOF){
    status = OK;
  }

  return status;

}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

