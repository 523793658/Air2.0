#ifndef _DEBUG_PACKAGE_H_
#define _DEBUG_PACKAGE_H_

#include <vector>
#include <string>


enum class LexerState
{
	INIT_STATE,
	IN_LENGTH_STATE,
	BEFORE_CONTENT_STATE,
	IN_CONTENT_STATE,
	END_STATE
};

class IRecver
{
public:
	virtual void RecvCallback(const char* data, int length) = 0;
};

class DebugPackage
{
public:
	DebugPackage();
	~DebugPackage();

	void Push(const char* data, int length);
	void SetRecver(IRecver* recver);

private:
	void HandleInitState(const char* data, int length);
	void HandleInLengthState(const char* data, int length);
	void HandleBeforeContentState(const char* data, int length);
	void HandleInContentState(const char* data, int length);
	void HandleEndState(const char* data, int length);


private:
	IRecver*  recver_;
	std::vector<char> recv_package_;
	std::vector<char> length_data_;
	int exist_len_;
	int package_len_;
	LexerState state_;

};

#endif // !_DEBUG_PACKAGE_H_
