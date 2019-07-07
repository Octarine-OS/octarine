#ifndef IO_DEVICE_HPP
#define IO_DEVICE_HPP

#include <stdint.h>

namespace io {

class Device {
  public:
	virtual ~Device() = default;
	virtual int ioctl(uint32_t req, void* arg) = 0;
};

} // namespace io
#endif
