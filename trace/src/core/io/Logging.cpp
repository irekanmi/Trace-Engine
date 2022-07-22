#include "Logging.h"

namespace trace {

	Exception::Exception(int line, std::string errdesc, std::string function, std::string file)
		: Line(line), mErrDesc(errdesc), mFunction(function), mFile(file)
	{

		std::stringstream Buffer;

		Buffer << "File : " << mFile << "\n";
		Buffer << "Function : " << mFunction << "\n";
		Buffer << "Line : " << Line << "\n";
		Buffer << "Error : " << mErrDesc << "\n\n";

		mErrText = Buffer.str();


	}

	Exception::~Exception() {}

	const char* Exception::what() const
	{
		return mErrText.c_str();
	}

}