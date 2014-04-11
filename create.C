#include "catalog.h"


const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
  Status status;
  RelDesc rd;
  AttrDesc ad;
  int offset, i;

  if (relation.empty() || attrCnt < 1)
    return BADCATPARM;

  if (relation.length() >= sizeof rd.relName)
    return NAMETOOLONG;

	if((status = getInfo(relation, rd)) != OK){
		return status;
	}

	// ? strcpy(rd.relName, relation.c_str());
	rd.attrCnt = attrCnt;

	if((status = addInfo(rd)) != OK){
		return status;
	}

	offset = 0;
	strcpy(ad.relName, relation.c_str());
	
	for(i = 0; i < attrCnt; i++){
		strcpy(ad.attrName, attrList[i].attrName);
		ad.attrType = attrList[i].attrType;
		ad.attrLen = attrList[i].attrLen;
		offset = offset + attrList[i].attrLen;

		if((status = attrCat->addInfo(ad)) != OK){
			return status;
		}
	}

	status = createHeapFile(relation);

	return status;

}