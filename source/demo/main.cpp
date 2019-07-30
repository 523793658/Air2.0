#include "RenderingThread.h"
#include "Containers/Array.h"


int main(int argc, char* argv[])
{
	Air::TArray<std::shared_ptr<int>> t;
	std::shared_ptr<int> a = std::make_shared<int>(2);
	t.add(a);
	std::cout << "dddd" << std::endl;
}