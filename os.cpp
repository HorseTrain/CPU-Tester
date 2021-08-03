#include "OS.H"

using namespace LibOS::IO;
using namespace LibOS::Graphics;

int main()
{
	InFileStream* test = new InFileStream("file.test");

	delete test;

	return -1;
};