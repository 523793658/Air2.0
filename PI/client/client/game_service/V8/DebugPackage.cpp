#include "DebugPackage.h"

DebugPackage::DebugPackage()
: package_len_(0)
, exist_len_(0)
, state_(LexerState::INIT_STATE)
{

}

DebugPackage::~DebugPackage()
{
}

void DebugPackage::Push(const char* data, int length)
{
	switch (state_)
	{
		case LexerState::INIT_STATE:
			HandleInitState(data, length);
			break;
		case LexerState::IN_LENGTH_STATE:
			HandleInLengthState(data, length);
			break;
		case LexerState::BEFORE_CONTENT_STATE:
			HandleBeforeContentState(data, length);
			break;
		case LexerState::IN_CONTENT_STATE:
			HandleInContentState(data, length);
			break;
		case LexerState::END_STATE:
			HandleEndState(data, length);
			break;
		default:
			break;
	}
}

void DebugPackage::SetRecver(IRecver* recver)
{
	recver_ = recver;
}

void DebugPackage::HandleInitState(const char* data, int length)
{
	std::string str = data;
	std::string::size_type index = str.find(':');
	if (index != std::string::npos)
	{
		state_ = LexerState::IN_LENGTH_STATE;
		if (length > static_cast<int>(index + 1))
		{
			HandleInLengthState(data + index + 1, length - index - 1);
		}
	} 
}


void DebugPackage::HandleInLengthState(const char* data, int length)
{
	std::string str = data;
	std::string::size_type index = str.find('\r');

	for (size_t i = 0; i < index; i++)
	{
		length_data_.push_back(str[i]);
	}

	if (index != std::string::npos)
	{
		state_ = LexerState::BEFORE_CONTENT_STATE;

		std::string len;
		len.assign(length_data_.begin(), length_data_.end());
		package_len_ = std::stoi(len);

		recv_package_.resize(package_len_, 0);

		if (length > static_cast<int>(index + 1))
		{
			HandleBeforeContentState(data + index, length - index);
		}
	}
}

void DebugPackage::HandleBeforeContentState(const char* data, int length)
{
	std::string str = data;
	std::string::size_type index = str.find('{');

	if (index != std::string::npos)
	{
		state_ = LexerState::IN_CONTENT_STATE;

		if (length > static_cast<int>(index + 1))
		{
			HandleInContentState(data + index, length - index);
		}
	}
}

void DebugPackage::HandleInContentState(const char* data, int length)
{
	int lack_len = package_len_ - exist_len_;

	if (length >= lack_len)
	{
		memcpy(&recv_package_[exist_len_], data, lack_len);

		state_ = LexerState::END_STATE;
		exist_len_ += lack_len;

		HandleEndState(data + lack_len, length - lack_len);
	} 
	else
	{
		memcpy(&recv_package_[exist_len_], data, length);
		exist_len_ += length;
	}
}

void DebugPackage::HandleEndState(const char* data, int length)
{

	std::string result;
	int result_len = package_len_;

	result.assign(recv_package_.begin(), recv_package_.end());
	recver_->RecvCallback(result.c_str(), result_len);

	package_len_ = 0;
	exist_len_ = 0;
	state_ = LexerState::INIT_STATE;
	length_data_.clear();
	recv_package_.clear();
}
