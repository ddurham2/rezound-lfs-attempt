/* 
 * Copyright (C) 2002 - David W. Durham
 * 
 * This file is not part of any particular application.
 * 
 * CNestedDataFile is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 * 
 * CNestedDataFile is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */


/*
 * I could have used XML, to store this sort of information, but it's way too bulky for me.
 * So, here's my simplistic solution.
 * -- Davy
 */

/* ??? I could consider using string instead of any char *'s passing strings seems to be quote efficient */

#include "CNestedDataFile.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <stdexcept>
#include <algorithm>

#include <istring>

extern int cfg_parse();

CNestedDataFile *CNestedDataFile::parseTree;
const char *CNestedDataFile::initialFilename;

// changed from "." because this is more likely not needed in a 
// preset's name, where "." might be quite useful in a preset name
const char *CNestedDataFile::delimChar="|"; // !!!NOTE!!! Change the qualified_ident rule in cfg.y also!!!

CNestedDataFile::CNestedDataFile(const string _filename,bool _saveOnEachEdit) :
	root(NULL),
	saveOnEachEdit(_saveOnEachEdit)
{
	clear();
	if(_filename!="")
		parseFile(_filename);
}

CNestedDataFile::~CNestedDataFile()
{
	delete root;
}

void CNestedDataFile::clear()
{
	delete root;
	root=new CVariant("root");
}

void CNestedDataFile::parseFile(const string _filename,bool clearExisting)
{
	if(clearExisting)
	{
		clear();
	}

	#ifdef THREAD_SAFE_CSCOPE
	mutex.lock();
	#endif
	try
	{
		initialFilename=_filename.c_str();
		filename=_filename;
		parseTree=this;

		if(cfg_parse())
			throw(runtime_error(string(__func__)+" -- error while parsing file: "+_filename));

		//free(scanString);
		#ifdef THREAD_SAFE_CSCOPE
		mutex.unlock();
		#endif
	}
	catch(...)
	{
		//free(scanString);
		#ifdef THREAD_SAFE_CSCOPE
		mutex.unlock();
		#endif

		if(clearExisting)
			clear();

		throw;
	}
}

void CNestedDataFile::setFilename(const string _filename)
{
	filename=_filename;
}


const string CNestedDataFile::getValue(const char *key,bool throwIfNotExists) const
{
	CVariant *value;
	if(!findVariantNode(value,key,0,true,root))
	{
		if(throwIfNotExists)
			throw(runtime_error(string(__func__)+" -- key '"+string(key)+"' does not exist from file: "+filename));
		else
			return("");
	}

	switch(value->type)
	{
	case ktString:
		return(value->stringValue);
	case ktFloat:
		return(istring(value->floatValue));
	case ktScope:
		throw(runtime_error(string(__func__)+" -- '"+string(key)+"' resolves to a scope, not a value from file: "+filename));
	default:
		throw(runtime_error(string(__func__)+" -- internal error: unhandled type: '"+istring(value->type)+"' from file: "+filename));
	}
}

const string CNestedDataFile::getArrayValue(const char *key,size_t index,bool throwIfNotExists) const
{
	CVariant *value;
	if(!findVariantNode(value,key,0,true,root))
	{
		if(throwIfNotExists)
			throw(runtime_error(string(__func__)+" -- key '"+string(key)+"' does not exist from file: "+filename));
		else
			return("");
	}

	if(value->type!=ktArray)
	{
		if(throwIfNotExists)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"' is not an array from file: "+filename));
		else
			return("");
	}

	if(index>=value->arrayValue.size())
	{
		if(throwIfNotExists)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"["+istring(index)+"]' array index is out of bounds (+"+istring(value->arrayValue.size())+"+) from file: "+filename));
		else
			return("");
	}

	switch(value->arrayValue[index].type)
	{
	case ktString:
		return(value->arrayValue[index].stringValue);
	case ktFloat:
		return(istring(value->arrayValue[index].floatValue));
	default:
		throw(runtime_error(string(__func__)+" -- internal error: unhandled type: '"+istring(value->arrayValue[index].type)+"' from file: "+filename));
	}
}

const size_t CNestedDataFile::getArraySize(const char *key,bool throwIfNotExists) const
{
	CVariant *value;
	if(!findVariantNode(value,key,0,true,root))
	{
		if(throwIfNotExists)
			throw(runtime_error(string(__func__)+" -- key '"+string(key)+"' does not exist from file: "+filename));
		else
			return(0);
	}

	if(value->type!=ktArray)
	{
		if(throwIfNotExists)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"' is not an array from file: "+filename));
		else
			return(0);
	}

	return(value->arrayValue.size());
}

CNestedDataFile::KeyTypes CNestedDataFile::keyExists(const char *key) const
{
	CVariant *value;
	if(findVariantNode(value,key,0,false,root))
		return(value->type);
	else
		return(ktNotExists);
}

const vector<string> CNestedDataFile::getChildKeys(const char *parentKey,bool throwIfNotExists) const
{
	vector<string> childKeys;
	CVariant *scope;

	if(parentKey==NULL || parentKey[0]==0)
		scope=root;
	else
	{
		if(!findVariantNode(scope,parentKey,0,true,root))
		{
			if(throwIfNotExists)
				throw(runtime_error(string(__func__)+" -- parent key '"+string(parentKey)+"' does not exist from file: "+filename));
			else
				return(childKeys);
		}

		if(scope->type!=ktScope)
		{
			if(throwIfNotExists) // it DID actually exist, but it wasn't a scope containing more child values
				throw(runtime_error(string(__func__)+" -- parent key '"+string(parentKey)+"' resolved to a value from file: "+filename));
			else
				return(childKeys);

		}
	}
	
	for(map<string,CVariant>::const_iterator i=scope->members.begin();i!=scope->members.end();i++)
		//childKeys.push_back(scope==root ? i->first.substr(1) : i->first);
		childKeys.push_back(i->first);
	
	return(childKeys);
}

bool CNestedDataFile::findVariantNode(CVariant *&retValue,const char *key,int offset,bool throwOnError,const CVariant *variant) const
{
	if(variant->type!=ktScope)
	{
		if(throwOnError)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"' resolves too soon to a value at: '"+string(key,max(0,offset-1))+"' from file: "+filename));
		else
			return(false);
	}

	// look for a dot in the key
	int pos=strchr(key+offset,*delimChar)-(key+offset);
	if(pos<0)
	{ // no dot found, then we must be now asking for a value
		map<string,CVariant>::const_iterator i=variant->members.find(string(key+offset));
		if(i==variant->members.end())
			return(false);
		else
		{
			retValue=const_cast<CVariant *>(&(i->second));
			return(true);
		}
	}
	else
	{ // dot found, then we must be asking for a nested scope
		map<string,CVariant>::const_iterator i=variant->members.find(string(key+offset,pos));
		if(i==variant->members.end())
			return(false);
		else
			return(findVariantNode(retValue,key,offset+pos+1,throwOnError,&i->second));
	}
}

#if 0
CNestedDataFile::CVariant *CNestedDataFile::upwardsScopeLookup(const char *key) const
{
/*
 * This method is used to resolve names like "aaa.bbb.c" when used in expressions
 * For example:
 * 	aaa
 * 	{
 * 		bbb
 * 		{
 * 			c=10;
 * 		}
 *
 * 		ddd
 * 		{
 * 			e=aaa.bbb.c;
 * 		}
 * 	}
 *
 * When The "aaa.bbb.c" token is encountered it should attempt to resolve that
 * name as a value.  The correct way to do this is: Starting within the scope, ddd
 * attempt to look up the name "aaa.bbb.c".  It won't be found, so go out a scope to
 * ddd's parent, aaa.  Then attempt to look up the name "aaa.bbb.c".  It again won't 
 * be found, so go out a scope again to aaa's parent, the root scope.  Then attempt
 * to look up the name "aaa.bbb.c".  This time it will resolve to the value, 10.
 *
 * The name upwardsScopeLookup means each time the lookup faile, it goes UP the 
 * scope tree to the searched scope's parent.
 */
	const CNestedDataFile *scope=this; // start with the current scope
	while(scope!=NULL)
	{
		CVariant *value;
		if(scope->findVariantNode(value,key,0,false))
			return(value);

		scope=scope->parentScope;
	}
	// not found -> exception on error???
	return(NULL);
}
#endif

void CNestedDataFile::createKey(const char *key,const double value)
{
	CVariant newVariant("",value);
	prvCreateKey(key,0,newVariant,root);

	if(saveOnEachEdit)
		save();
}

void CNestedDataFile::createKey(const char *key,const string &value)
{
	CVariant newVariant("",value);
	prvCreateKey(key,0,newVariant,root);

	if(saveOnEachEdit)
		save();
}

void CNestedDataFile::createArrayKey(const char *key,size_t index,const string &initValue)
{
	CVariant *value;
	if(!findVariantNode(value,key,0,true,root))
	{
		if(index!=0)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"["+istring(index)+"]' key not found from file: "+filename));

		vector<CVariant> values;
		values.push_back(CVariant("",initValue));

		CVariant newVariant(values);
		prvCreateKey(key,0,newVariant,root);
		return;
	}

	if(value->type!=ktArray)
		throw(runtime_error(string(__func__)+" -- '"+string(key)+"' is not an array from file: "+filename));

	if(index>value->arrayValue.size())
		throw(runtime_error(string(__func__)+" -- '"+string(key)+"["+istring(index)+"]' array index is out of bounds (+"+istring(value->arrayValue.size())+"+) from file: "+filename));

	value->arrayValue.insert(value->arrayValue.begin()+index,CVariant("",initValue));

	if(saveOnEachEdit)
		save();
}

void CNestedDataFile::createKey(const char *key,const vector<CVariant> &value)
{
	CVariant newVariant(value);
	prvCreateKey(key,0,newVariant,root);

	if(saveOnEachEdit)
		save();
}

void CNestedDataFile::removeKey(const char *key,bool throwOnError)
{
	CVariant *parent=NULL;;

	string parentKey;
	string childKey;
	size_t lastDot=string(key).rfind(*delimChar);
	if(lastDot==string::npos)
	{
		parent=root;
		childKey=key;
	}
	else
	{
		parentKey=string(key).substr(0,lastDot);
		childKey=string(key).substr(lastDot+1);
	}

	if(parent==root || findVariantNode(parent,parentKey.c_str(),0,throwOnError,root))
	{
		if(parent->members.find(childKey)==parent->members.end())
		{
			if(throwOnError)
				throw(runtime_error(string(__func__)+" -- '"+string(key)+"' key not found from file: "+filename));
			return;
		}
		parent->members.erase(childKey);
	}

	if(saveOnEachEdit)
		save();
}

void CNestedDataFile::removeArrayKey(const char *key,size_t index,bool throwOnError)
{
	CVariant *value;
	if(!findVariantNode(value,key,0,true,root))
	{
		if(throwOnError)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"["+istring(index)+"]' key not found from file: "+filename));
		return;
	}

	if(value->type!=ktArray)
	{
		if(throwOnError)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"' is not an array from file: "+filename));
		return;
	}

	if(index>=value->arrayValue.size())
	{
		if(throwOnError)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"["+istring(index)+"]' array index is out of bounds (+"+istring(value->arrayValue.size())+"+) from file: "+filename));
		return;
	}

	value->arrayValue.erase(value->arrayValue.begin()+index);

	if(saveOnEachEdit)
		save();
}


void CNestedDataFile::prvCreateKey(const char *key,int offset,CVariant &value,CVariant *variant)
{
	if(variant==root) // only verify once
		verifyKey(key);

	// look for a dot in the key
	int pos=strchr(key+offset,*delimChar)-(key+offset);
	if(pos<0)
	{ // no dot found, then we must be now creating a value
		//printf("creating new value: %s\n",string(key+offset).c_str());
		value.name=string(key+offset);
		variant->members[string(key+offset)]=value;
	}
	else
	{ // dot found, then we must be asking for a nested scope

		if(variant->type!=ktScope)
			throw(runtime_error(string(__func__)+" -- '"+string(key)+"' resolves too soon to a value at: '"+string(key,offset+pos)+"' from file: "+filename));

		map<string,CVariant>::const_iterator i=variant->members.find(string(key+offset,pos));
		if(i==variant->members.end())
		{ // create a new scope
			//printf("creating new scope: %s\n",string(key+offset,pos).c_str());
			variant->members[string(key+offset,pos)]=CVariant(string(key+offset,pos));
			prvCreateKey(key,offset+pos+1,value,&variant->members[string(key+offset,pos)]);
		}
		else
		{
			//printf("while creating key, decending into: %s\n",string(key+offset,pos).c_str());
			prvCreateKey(key,offset+pos+1,value,const_cast<CVariant *>(&(i->second)));
		}
	}
}

void CNestedDataFile::save() const
{
	writeFile(filename);
}

void CNestedDataFile::writeFile(const string filename) const
{
	if(filename=="")
		return;

	// ??? do I wanna reassign this->filename ?
	FILE *f=fopen(filename.c_str(),"wt");
	if(f==NULL)
		throw(runtime_error(string(__func__)+" -- error opening file for write: "+filename));

	fprintf(f,"// ReZound program generated data; be careful if modifying.  Consider making a backup before modifying!\n\n");

	try
	{
		prvWriteData(f,-1,root);
		fclose(f);
	}
	catch(...)
	{
		fclose(f);
		throw;
	}
}

/* translate \ to \\ and " to \" in the given filename */
static const string fixEscapes(const string _s)
{
	string s=_s;
	for(size_t t=0;t<s.size();t++)
	{
		if(s[t]=='\\' || s[t]=='"')
		{
			s.insert(t,"\\");
			t++;
		}
	}
	return(s);
}

void CNestedDataFile::prvWriteData(void *_f,int indent,const CVariant *variant) const
{
	FILE *f=(FILE *)_f; // to avoid including stdio.h in the .h file

	for(int t=0;t<indent;t++)
		fprintf(f,"\t");

	string name=variant->name;

	// convert non-alnum to '\'non-alnum (and also don't make '_' into '\_')
	for(size_t t=0;t<name.length();t++)
	{
		if((!isalnum(name[t]) && name[t]!='_') || (t==0 && isdigit(name[t])))
		{
			name.insert(t,"\\");
			t++;
		}
	}

	switch(variant->type)
	{
	case ktString:
		fprintf(f,"%s=\"%s\";\n",name.c_str(),fixEscapes(variant->stringValue).c_str());
		break;

	case ktFloat:
		// ??? I may want to do a better job of making sure I don't truncate any necessary percision on outputing the value
		fprintf(f,"%s=%f;\n",name.c_str(),variant->floatValue);
		break;

	case ktScope:
	{
		if(indent>=0) // not root scope
		{
			fprintf(f,"%s\n",name.c_str());

			for(int t=0;t<indent;t++)
				fprintf(f,"\t");
			fprintf(f,"{\n");
		}

		// write non scopes for (just to look nicer)
		bool more=false;
		for(map<string,CVariant>::const_iterator i=variant->members.begin();i!=variant->members.end();i++)
		{
			if(i->second.type!=ktScope)
				prvWriteData(f,indent+1,&i->second);
			else
				more=true;
		}
		if(more) // there will be scopes written
			fprintf(f,"\n");

		// write scopes after non-scopes
		for(map<string,CVariant>::const_iterator i=variant->members.begin();i!=variant->members.end();i++)
		{
			if(i->second.type==ktScope)
				prvWriteData(f,indent+1,&i->second);
		}

		if(indent>=0) // not root scope
		{
			for(int t=0;t<indent;t++)
				fprintf(f,"\t");
			fprintf(f,"}\n");
			if(indent==0)
				fprintf(f,"\n");
		}
		break;
	}

	case ktArray:
		fprintf(f,"%s[]={",name.c_str());
		for(size_t t=0;t<variant->arrayValue.size();t++)
		{
			if(t!=0)
				fprintf(f,", ");
			switch(variant->arrayValue[t].type)
			{
			case ktString:
				fprintf(f,"\"%s\"",variant->arrayValue[t].stringValue.c_str());
				break;
			case ktFloat:
				fprintf(f,"%f",variant->arrayValue[t].floatValue);
				break;
			default:
				throw(runtime_error(string(__func__)+" -- internal error: unhandled type while writing array: "+istring(variant->arrayValue[t].type)+" from file: "+filename));
			}
		}
		fprintf(f,"};\n");
		break;

	default:
		throw(runtime_error(string(__func__)+" -- internal error: unhandled type: "+istring(variant->type)+" from file: "+filename));
	}
}

/*
 * This method makes sure that when creating a key in the file that it 
 * does not contain invalid characters that would cause a parse error
 * if the file were parsed again with this supposed invalid key.
 */
void CNestedDataFile::verifyKey(const char *key)
{
	size_t l=strlen(key);
	for(size_t t=0;t<l;t++)
	{
		/*
		if((!isalnum(key[t]) && key[t]!=' ' && key[t]!=':' && key[t]!='_' && key[t]!='.') || ((t==0||key[t-1]=='.') && isdigit(key[t]))) 
			throw(runtime_error(string(__func__)+" -- invalid character in key: '"+key+"' or first character of a sub-key is a digit for creating key in file: "+filename));
		*/
		/*
		if(!isalnum(key[t]) && key[t]!=' ' && key[t]!=':' && key[t]!='_' && key[t]!='.') 
		*/
			// ??? This totally depends on this code running on an ASCII machine
		if(key[t]<32 || key[t]>126)
			throw(runtime_error(string(__func__)+" -- invalid character in key: '"+key+"' creating key in file: "+filename));
	}
}




// -----------------------------------------------------------------------------

CNestedDataFile::CVariant::CVariant() :
	type(ktNotExists)
{
}

CNestedDataFile::CVariant::CVariant(const string _name) :
	name(_name),
	type(ktScope)
{
}

CNestedDataFile::CVariant::CVariant(const string _name,const string value) :
	name(_name),
	type(ktString),
	stringValue(value)
{
	if(name!="")
		printf("okay.. it is used: %s\n",name.c_str());
}

CNestedDataFile::CVariant::CVariant(const string _name,const double value) :
	name(_name),
	type(ktFloat),
	floatValue(value)
{
	if(name!="")
		printf("okay.. it is used: %s\n",name.c_str());
}

CNestedDataFile::CVariant::CVariant(const vector<CVariant> &value) :
	type(ktArray),
	arrayValue(value)
{
}

CNestedDataFile::CVariant::CVariant(const CVariant &src) :
	type(ktNotExists)
{
	operator=(src);
}

const CNestedDataFile::CVariant &CNestedDataFile::CVariant::operator=(const CVariant &rhs)
{
	if(this==&rhs)	
		return(*this);

	name=rhs.name;
	type=rhs.type;

	if(type==ktScope)
		members=rhs.members;
	else if(type==ktString)
		stringValue=rhs.stringValue;
	else if(type==ktFloat)
		floatValue=rhs.floatValue;
	else if(type==ktArray)
		arrayValue=rhs.arrayValue;

	return(*this);
}

CNestedDataFile::CVariant::~CVariant()
{
}
