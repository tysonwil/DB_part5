#include "catalog.h"

//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::destroyRel(const string & relation)
{
  Status status;

  if (relation.empty() || 
      relation == string(RELCATNAME) || 
      relation == string(ATTRCATNAME))
    return BADCATPARM;

	if ((status = attrCat->dropRelation(relation)) != OK) {
		return status;
	}
	if((status = removeInfo(relation)) != OK){
		return status;
	}
	if((status = destroyHeapFile(relation)) != OK){
		return status;
	}
	return OK;
}


//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{
  Status status;
  AttrDesc *attrs;
  int attrCnt, i;

  if (relation.empty()) return BADCATPARM;

  if((status = attrCat->getRelInfo(relation, attrCnt, attrs)) != OK) {
    return status;
  }

  for(i = 0; i < attrCnt; i++){
    if((status = attrCat->removeInfo(relation, attrs[i].attrName)) != OK){
      if(status == RELNOTFOUND) return OK;
    	return status;
    }
  }
  return status;
}


