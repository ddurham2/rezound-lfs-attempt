#include "../../../config/common.h"

#include <stdio.h>

#include "CNestedDataFile.h"

int main()
{
	try
	{
		CNestedDataFile s("test.dat");

		s.createKey("foo.bar.bill.ester","15");
		s.createKey("foo.x",200.123);

		printf("jim.size %u\n",s.getArraySize("jim"));
		printf("%s\n",s.getArrayValue("jim",2).c_str());

		{
		const vector<string> v=s.getChildKeys("foo");
		for(size_t t=0;t<v.size();t++)
			printf("foo.%s\n",v[t].c_str());
		}

		{
		const vector<string> v=s.getChildKeys(NULL);
		for(size_t t=0;t<v.size();t++)
			printf("root: '%s'\n",v[t].c_str());
		}

		s.writeFile("test.dat");

	}
	catch(exception &e)
	{
		fprintf(stderr,"exception caught -- %s\n",e.what());
	}
	catch(...)
	{
		fprintf(stderr,"unknown exception caught\n");
	}

	return(0);
}
