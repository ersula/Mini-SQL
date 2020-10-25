#include "h/RecordManage.h"

void RecordManage::CreateTable(table & t)
{
	const string FileName = t.tablename + ".table";
	fstream fout(FileName.c_str(), ios::out);
	fout.close();
}

void RecordManage::DropTable(table & t)
{
	const string FileName = t.tablename + ".table";
	if (remove(FileName.c_str()))
	{
		perror("File deletion failed!");
	}
	else
	{
		bm.deleteFile(FileName);
	}
}

tag RecordManage::InsertRecord(table & t, vector<element> colomns)
{
	string record = transtostring(t, colomns);
	int insertBufferIndex = -1;
	int insertDataPosition = -1;
	if (t.BlockCount == 0)
	{
		insertBufferIndex = mallocBlockInTableFile(t);
		insertDataPosition = 0;
	}
	else
	{
		string fileName = t.tablename + ".table";
		int tupleLength = t.EntrySize + 1;
		int blockIndex = t.BlockCount - 1;

		int bufferIndex = bm.checkInBuffer(fileName, blockIndex);
		if (bufferIndex == -1)
		{
			bufferIndex = bm.GetEmptyBuffer();
			bm.BufferRead(fileName, blockIndex, bufferIndex);
		}
		int maxTupleNum = OneBlockSize / tupleLength;
		for (int i = 0; i < maxTupleNum; i++)
		{
			int dataPosition = i * tupleLength;
			char data = bm.bufferBlock[bufferIndex].data[dataPosition];
			if (data == EMPTY)
			{
				insertBufferIndex = bufferIndex;
				insertDataPosition = dataPosition;
				break;
			}
		}
		if (insertDataPosition == -1 && insertBufferIndex == -1)
		{
			insertBufferIndex = mallocBlockInTableFile(t);
			insertDataPosition = 0;
		}
	}
	bm.bufferBlock[insertBufferIndex].data[insertDataPosition] = 'A';
	for (int i = 0; i < t.EntrySize; i++)
	{
		bm.bufferBlock[insertBufferIndex].data[insertDataPosition + i + 1] = record.c_str()[i];
	}
	bm.bufferBlock[insertBufferIndex].Dirty = true;
	bm.RecordBuffer(insertBufferIndex);
	bm.writeToFile(insertBufferIndex);
	tag T = make_pair(insertBufferIndex, insertDataPosition);
	return T;
}

Data RecordManage::select(table & t)
{
	Data selectResult;
	string fileName = t.tablename + ".table";
	int tupleLength = t.EntrySize + 1;
	int maxTupleNum = OneBlockSize / tupleLength;
	for (int blockIndex = 0; blockIndex < t.BlockCount; blockIndex++)
	{
		int bufferIndex = bm.checkInBuffer(fileName, blockIndex);
		if (bufferIndex == -1)
		{
			bufferIndex = bm.GetEmptyBuffer();
			bm.BufferRead(fileName, blockIndex, bufferIndex);
		}
		for (int i = 0; i < maxTupleNum; i++)
		{
			int dataPosition = i * tupleLength;
			string stringTuple = bm.bufferBlock[bufferIndex].readBlockSeg(dataPosition, dataPosition + tupleLength);
			if (stringTuple.c_str()[0] != EMPTY)
			{
				stringTuple.erase(stringTuple.begin());
				vector<element> tuple = transtoelement(t, stringTuple);
				/*cout << "The element back to API:" << endl;
				for (int i = 0; i < tuple.size(); i++) {
					cout << tuple[i].type << " " << tuple[i].i << " " << tuple[i].f << " " << tuple[i].s << endl;
				}*/
				selectResult.rows.push_back(tuple);
			}
		}
	}
	return selectResult;
}

vector<element> RecordManage::select(table & t, tag T)
{
	int tupleLength = t.EntrySize + 1;
	int bufferIndex = T.first;
	int dataPosition = T.second;
	string stringTuple = bm.bufferBlock[bufferIndex].readBlockSeg(dataPosition, dataPosition + tupleLength);
	if (stringTuple.c_str()[0] != EMPTY)
	{
		stringTuple.erase(stringTuple.begin());
		vector<element> tuple = transtoelement(t, stringTuple);
		return tuple;
	}
}

void RecordManage::Delete(table & t, tag T)
{
	int tupleLength = t.EntrySize + 1;
	int bufferIndex = T.first;
	int dataPosition = T.second;
	string stringTuple = bm.bufferBlock[bufferIndex].readBlockSeg(dataPosition, dataPosition + tupleLength);
	if (stringTuple.c_str()[0] != EMPTY)
	{
		//stringTuple.erase(stringTuple.begin());
		bm.bufferBlock[bufferIndex].data[dataPosition] = EMPTY;
	}
	bm.writeToFile(bufferIndex);
	bm.bufferBlock[bufferIndex].Dirty = true;
}

void RecordManage::Delete(table & t)
{
	string fileName = t.tablename + ".table";
	int tupleLength = t.EntrySize + 1;
	int maxTupleNum = OneBlockSize / tupleLength;
	for (int blockIndex = 0; blockIndex < t.BlockCount; blockIndex++)
	{
		int bufferIndex = bm.checkInBuffer(fileName, blockIndex);
		if (bufferIndex == -1)
		{
			bufferIndex = bm.GetEmptyBuffer();
			bm.BufferRead(fileName, blockIndex, bufferIndex);
		}
		for (int i = 0; i < maxTupleNum; i++)
		{
			int dataPosition = i * tupleLength;
			string stringTuple = bm.bufferBlock[bufferIndex].readBlockSeg(dataPosition, dataPosition + tupleLength);
			if (stringTuple.c_str()[0] != EMPTY)
			{
				bm.bufferBlock[bufferIndex].data[dataPosition] = EMPTY;
			}
		}
		bm.writeToFile(bufferIndex);
		bm.bufferBlock[bufferIndex].Dirty = true;
	}
}

vector<tag> RecordManage::FindAllTag(table & t)
{
	vector<tag> Offsets;
	string fileName = t.tablename + ".table";
	int tupleLength = t.EntrySize + 1;
	int maxTupleNum = OneBlockSize / tupleLength;
	for (int blockIndex = 0; blockIndex < t.BlockCount; blockIndex++)
	{
		int bufferIndex = bm.checkInBuffer(fileName, blockIndex);
		if (bufferIndex == -1)
		{
			bufferIndex = bm.GetEmptyBuffer();
			bm.BufferRead(fileName, blockIndex, bufferIndex);
		}
		for (int i = 0; i < maxTupleNum; i++)
		{
			int dataPosition = i * tupleLength;
			string stringTuple = bm.bufferBlock[bufferIndex].readBlockSeg(dataPosition, dataPosition + tupleLength);
			if (stringTuple.c_str()[0] != EMPTY)
			{
				/*cout << "The element back to API:" << endl;
				for (int i = 0; i < tuple.size(); i++) {
					cout << tuple[i].type << " " << tuple[i].i << " " << tuple[i].f << " " << tuple[i].s << endl;
				}*/
				tag T = make_pair(bufferIndex, dataPosition);
				Offsets.push_back(T);
			}
		}
	}
	return Offsets;
}

string RecordManage::transtostring(table &t, vector<element> colomns)
{
	string str;
	for (int i = 0; i < colomns.size(); i++) {
		string part;
		stringstream ss;
		switch (colomns[i].type)
		{
		case 0:
			ss << colomns[i].i;
			ss >> part;
			while (part.length() < sizeof(int)) part += EMPTY;
			if (part.length() > sizeof(int)) t.attr[i].charnum = part.length();
			break;
		case 1:
			ss << colomns[i].f;
			ss >> part;
			while (part.length() < sizeof(float)) part += EMPTY;
			if (part.length() > sizeof(float)) t.attr[i].charnum = part.length();
			break;
		case 2:
			ss << colomns[i].s;
			ss >> part;
			while (part.length() < t.attr[i].charnum) part += EMPTY;
			break;
		default:
			break;
		}
		str += part;
	}
	return str;
}

vector<element> RecordManage::transtoelement(table & t, string str)
{
	vector<element>record;
	string attributeValue;
	int start = 0, end = 0;
	for (int i = 0; i < t.attr.size(); i++) {
		start = end;
		attributeValue = "";
		
		end += t.attr[i].charnum;

		for (int j = start; j < end; j++) attributeValue += str.c_str()[j];

		element a;
		switch (t.attr[i].type)
		{
		case 0:
			a.i = atoi(attributeValue.c_str());
			a.type = 0;
			break;
		case 1:
			a.f = atof(attributeValue.c_str());
			a.type = 1;
			break;
		case 2:
			a.s = attributeValue;
			a.type = 2;
			break;
		default:
			break;
		}
		record.push_back(a);
	}
	return record;
}

int RecordManage::mallocBlockInTableFile(table & t)
{
	int bufferIndex = bm.GetEmptyBuffer();
	bm.bufferBlock[bufferIndex].Dirty = true;
	bm.bufferBlock[bufferIndex].isEmpty = false;
	bm.bufferBlock[bufferIndex].fileName = t.tablename + ".table";
	bm.bufferBlock[bufferIndex].blockIndex = t.BlockCount++;
	bm.RecordBuffer(bufferIndex);
	return bufferIndex;
}
