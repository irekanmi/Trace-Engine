#pragma once

#include "serialize/DataStream.h"

namespace trace::Network {


	class NetworkStream : public DataStream
	{

	public:
		virtual void Read(void* data, uint32_t size) override;
		virtual void Write(void* data, uint32_t size) override;
		virtual void SetPosition(uint32_t pos) override;
		virtual uint32_t GetPosition() override;

	private:
	protected:

	};

}
