
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

		try
		{
			TRC_EXCEPTION("Trace Exceptions");
		}
		catch (trace::Exception &ex)
		{
			printf("%s", ex.what());
		}
		
		try
		{
			TRC_EXCEPTION("Handle Case");
		}
		catch (trace::Exception &ex)
		{
			printf("%s", ex.what());
		}


		TRC_TRACE("Trace Engine %s", "in progress");
		TRC_DEBUG("Trace Engine %s", "in progress");
		TRC_INFO("Trace Engine %s", "in progress");
		TRC_WARN("Trace Engine %s", "in progress");
		TRC_ERROR("Trace Engine %s", "in progress");
		TRC_CRITICAL("Trace Engine %s", "in progress");

		std::cin.get();

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