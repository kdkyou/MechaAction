#include"KeyData.h"

const bool KeyData::GetMove(int dataIndex)
{
	/*if (m_dataes[dataIndex]||KeyBit::W && 
		m_dataes[dataIndex]=)
	{

	} */

	return false;
}

void KeyData::Update()
{
	UINT nowData = None;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		nowData = nowData | KeyBit::W;
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		nowData = nowData | KeyBit::A;
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		nowData = nowData | KeyBit::S;
	}
	
	if (GetAsyncKeyState('D') & 0x8000)
	{
		nowData = nowData | KeyBit::D;
	}

}
