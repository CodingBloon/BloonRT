#pragma once

namespace Core {

	class Buffer {
	public:
		void* getMapped();
		VkDeviceAddress getAddress();
	private:
		void* mapped = nullptr;
	};

}