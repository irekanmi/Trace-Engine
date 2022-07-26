
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

		trace::Logger* log = trace::Logger::get_instance();

		TRC_TRACE("Trace Engine %s", "in progress")

		trace::MemoryPoolLL<uint64_t> pool;

		pool.Reserve(3);

		uint64_t* value = pool.Malloc();

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