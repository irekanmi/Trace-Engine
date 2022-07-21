
#include <trace.h>
#include <stdio.h>


class TestApp : public  trace::Application
{

public:
	TestApp()
	{


	}
	~TestApp()
	{

	}

	virtual void Start() override
	{
		printf("Application Started\n");
	}

	virtual void Run() override
	{
		printf("Application Running\n");
	}

	virtual void End() override
	{
		printf("Application Ended\n");

	}


};


trace::Application* trace::CreateApp()
{

	return new TestApp();

}