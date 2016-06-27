#include "FEMElement.h"

FEMElement::FEMElement(_ElementsS elem)
{

	nodeCount = 0;

	elemID = elem.id;

	int n = (int)elem.nodeIDList.size();
		
	for (int i = 0; i < n; ++i)
	{
		int id = elem.nodeIDList[i];

		if ((nodeCount == 0) || (nodeIDs[(int)nodeIDs.size() - 1] != id))
		{
			if (n == 8)
				if (i > 3)
					if (nodeIDs[(int)nodeIDs.size() - 3] == id)
						continue;

			++nodeCount;
			nodeIDs.push_back(id);
		}
	}

	B.resize(3, 2 * nodeCount);

}

FEMElement::~FEMElement()
{
	
}