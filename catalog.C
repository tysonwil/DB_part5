#include "catalog.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <assert.h> 


RelCatalog::RelCatalog(Status &status) :
   HeapFile(RELCATNAME, status)
{
// nothing should be needed here
}

// Gets the information from the record catalog based on the relation and record
// Returns an OK status if the relation was already created and readable
const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
  if (relation.empty())
    return BADCATPARM;

  Status status;
  Record rec;
  RID rid;

  // Create a new HeapFileScan on relationship catalog

  HeapFileScan* heapScan;
  heapScan = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;

  // Look for the beginning of the relationship catalog using the HeapFileScan

  if((status = heapScan->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    delete heapScan;
    return status;
  }

  // Get the RID and Get the Record for the RelDesc once found

  status = heapScan->scanNext(rid);
  if (status == OK) {
    if ((status = heapScan->getRecord(rec)) != OK) 
      return status;
    memcpy(&record, rec.data, rec.length);
  }
  else if (status == FILEEOF)
    status = RELNOTFOUND;

  heapScan->endScan();

  // Delete HeapFileScan and return status
  
  delete heapScan;
  return status;
}


// Used to create the inital RelDesc for the table
// Return OK
const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;
  Record rec;

  // Initalize a new InsertFileScan

  ifs = new InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;

  // Create the Record data to insert the new RelDesc

  int length = strlen(record.relName);
  memset(&record.relName[length], 0, sizeof(record.relName - length));
  rec.data = &record;
  rec.length = sizeof(RelDesc);

  // Insert record and clean up

  status = ifs->insertRecord(rec, rid);
  delete ifs;
  return status;
}


// Remove the tuple correspond to the name of relName from the RelCat
// Returns OK status if successfully removes the relName
const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;

  // Initalize new HeapFileScan

  hfs = new HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;

  // Scan for the relName to point to the record

  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    return status;
  }

  // Scan for the RID of the record and delete it if found

  if ((status = hfs->scanNext(rid)) != OK) {
      if (status == NORECORDS)
        status = OK;
      else if (status == FILEEOF)
        status = RELNOTFOUND;
  }
  else
    status = hfs->deleteRecord();

  // Return Status and clean up HeapFileScan

  hfs->endScan();
  delete hfs;
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


// Returns (return param) the attribute descriptor record for the attribute name
// in the relation
// Returns OK status if it is successfully found and returned
const Status AttrCatalog::getInfo(const string & relation, 
          const string & attrName,
          AttrDesc &record)
{

  Status status;
  RID rid;
  Record rec;
  HeapFileScan*  hfs;

  if (relation.empty() || attrName.empty()) return BADCATPARM;

  // Initalize a HeapFileScan and find the start of the relation

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    delete hfs;
    return status;
  }

  // Scan the relationship until the correct file is found or an
  // error status occurs

  while ((status = hfs->scanNext(rid)) == OK) {
    if((status = hfs->getRecord(rec)) != OK)
      return status;

    memcpy(&record, rec.data, rec.length);
    if (strcmp(record.attrName, attrName.c_str()) == 0)
        break;
  }
  if (status == FILEEOF)
    status = ATTRNOTFOUND;
  
  // Return status and clean up

  hfs->endScan();
  delete hfs;
  return status;
}


// Adds a tuple into the attribute catalog relation
// Returns OK status if the tuple is added successfully
const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  Record rec;
  InsertFileScan*  ifs;
  Status status;
  int length;

  // Initalize a new InsertFileScan

  ifs = new InsertFileScan(ATTRCATNAME, status);
  if (status != OK) return status;

  // Initalize the memory locations with the attrName information

  length = strlen(record.relName);
  memset(&record.relName[length], 0, sizeof(record.relName - length));
  length = strlen(record.attrName);
  memset(&record.attrName[length], 0, sizeof(record.attrName - length));

  // Set the attributes of the record with the data of record

  rec.data = &record;
  rec.length = sizeof(AttrDesc);
  status = ifs->insertRecord(rec, rid);

  // Return status and clean up

  delete ifs;
  return status;
}


// Removes a tuple from the attribute catalog that matches the attribute
// of the relation
// Returns OK when remove successfully
const Status AttrCatalog::removeInfo(const string & relation, 
             const string & attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record;
  HeapFileScan*  hfs;

  if (relation.empty() || attrName.empty()) return BADCATPARM;

  // Initalize a new HeapFileScan

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;

  // Scan for the start of the relationship

  if((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK){
    delete hfs;
    return status;
  }

  // Scan records until a matching tuple is found or a bad status

  while ((status = hfs->scanNext(rid)) == OK) {
    if((status = hfs->getRecord(rec)) != OK) {
      if(status == NORECORDS){
        status = OK;
      }
      return status;
    }

    memcpy(&record, rec.data, rec.length);

    // Delete the record once it is found

    if(strcmp(record.attrName, attrName.c_str()) == 0) {
      status = hfs->deleteRecord();
      break;
    }
  }

  // Return status and clean up
  
  hfs->endScan();
  delete hfs;
  return status;
}


// Returns the descriptions of all the attributes in the relation
// and the number of attributes in the relation
// Returns OK status when all attributes are able to return
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

  // Initalize a new HeapFileScan

  hfs = new HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;

  // Scan for the start of the relationship

  if ((status = hfs->startScan(0, relation.length() + 1, STRING, relation.c_str(), EQ)) != OK) {
    delete hfs;
    return status;
  }

  // Scan each attribute and allocate into the attrs array

  while ((status = hfs->scanNext(rid)) == OK) {
    if ((status = hfs->getRecord(rec)) != OK)
      return status;

    attrCnt++;
    if (attrCnt == 1) {
         if (!(attrs = (AttrDesc*)malloc(sizeof(AttrDesc))))
         return INSUFMEM;
    } 
    else {
      if (!(attrs = (AttrDesc*)realloc(attrs, attrCnt*sizeof(AttrDesc))))
         return INSUFMEM;
    }
    memcpy(&attrs[attrCnt - 1], rec.data, rec.length);
  }

  // Check case if there were no attributes in the relationship

  if (status == FILEEOF) {
    if (attrCnt == 0)
      status = RELNOTFOUND;
    else
      status = OK;
  }

  // Return status and clean up

  hfs->endScan();
  delete hfs;
  return status;
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

