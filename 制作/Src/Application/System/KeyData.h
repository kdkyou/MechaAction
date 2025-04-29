#pragma once

class KeyData {

public:

	KeyData(){}

	//入力キービット：入力されたキーに対応するビットを立てる
	enum KeyBit {
		None = 0,
		W = 1<<0,
		A = 1 << 1,
		S = 1 << 2,
		D = 1 << 3,
		Space = 1 << 4,
		Return = 1 << 5,
		LShift = 1 << 6,
		LButton = 1 << 7,
		RButton = 1 << 8,
	};

	//現在のデータを返す
	const UINT GetKeyDataNow(int dataIndex=0) { return m_dataes[dataIndex]; }

	const bool GetMove(int dataIndex = 0);
	const bool GetSpace();


	void Update();

	//キーデータの記録数：初期値10
	void SetLength(int _length) { m_maxLength = _length; }

private:

	//入力状況を表したキーデータのキュー
	std::deque<UINT> m_dataes;

	//何フレーム前までのデータを取るか
	UINT m_maxLength = 10;
	
};