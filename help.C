#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for the
// user. If no relation is given (relation is NULL), then it lists all
// the relations in the database, along with the width in bytes of the
// relation, the number of attributes in the relation, and the number of
// attributes that are indexed.  If a relation is given, then it lists
// all of the attributes of the relation, as well as its type, length,
// and offset, whether it's indexed or not, and its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt, i;

  if (relation.empty()) return UT_Print(RELCATNAME);

  // Get the relation catalog info for the relation

  if((status = getInfo(relation, rd)) != OK){
  	return status;
  }

  // Get the attribute catalog data for each attribute

  if((status = attrCat->getRelInfo(relation, attrCnt, attrs)) != OK){
  	return status;
  }

  // Print the relation and all of the attributes

  cout << "Relation: " << rd.relName << " Attributes: " << rd.attrCnt << endl;
  cout << "Attribute | Offset | Type | Length" << endl;
  for(i = 0; i < attrCnt; i++){
  	cout << attrs[i].attrName << " | ";
  	cout << attrs[i].attrOffset << " | ";
  	if(attrs[i].attrType == STRING){
  		cout << "STRING | ";
  	}
  	if(attrs[i].attrType == INTEGER){
  		cout << "INTEGER | ";
  	}
  	if(attrs[i].attrType == FLOAT){
  		cout << "FLOAT | ";
  	}
  	cout << attrs[i].attrLen << endl;
  } 

  return OK;
}
