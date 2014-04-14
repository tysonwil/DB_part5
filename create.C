#include "catalog.h"


// Creates a relational table using relation, attrCnt, attrList[]
// Returns an OK status if it was create successfully
const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
	Status status;
	RelDesc rd;
	AttrDesc ad;
	int offset, i, j, dataSize;

	if (relation.empty() || attrCnt < 1)
    	return BADCATPARM;

	if (relation.length() >= sizeof(rd.relName))
    	return NAMETOOLONG;

    // Does the relation already exist?

    status = getInfo(relation, rd);
    if (status == OK)
    	return RELEXISTS;
    else if (status != RELNOTFOUND)
    	return status;

    // Check for Data Size and Duplicate Attributes

    dataSize = 0;

    if(attrCnt > 1){
    	for(i = 0; i < attrCnt; i++){
    		dataSize += attrList[i].attrLen;
    		for(j = 0; j < i; j++){
    			if(strcmp(attrList[i].attrName, attrList[j].attrName) == 0){
    				return DUPLATTR;
    			}
    		}
    	}
    }

    // Date Size too Large

    if(dataSize > PAGESIZE){
    	return ATTRTOOLONG;
    }

    // Initalize RelDesc

    strcpy(rd.relName, relation.c_str());
	rd.attrCnt = attrCnt;

	if ((status = addInfo(rd)) != OK){
		return status;
	}

	offset = 0;
	
	// Copy in Attributes into AttrDesc

	for (i = 0; i < attrCnt; i++) {
		strcpy(ad.attrName, attrList[i].attrName);
		ad.attrType = attrList[i].attrType;
		ad.attrLen = attrList[i].attrLen;
		ad.attrOffset = offset;
		strcpy(ad.relName, attrList[i].relName);

		offset = offset + attrList[i].attrLen;

		if ((status = attrCat->addInfo(ad)) != OK)
			return status;
	}

	// Create HeapFile

	status = createHeapFile(relation);
	return status;
}