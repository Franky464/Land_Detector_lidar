#include "I2CDevice.h"

#include <AP_HAL/AP_HAL.h>

namespace PX4 {

class PX4_I2C : public device::I2C {
public:
    PX4_I2C(uint8_t bus);
    bool do_transfer(uint8_t address, const uint8_t *send, uint32_t send_len,
                     uint8_t *recv, uint32_t recv_len);
};

PX4_I2C::PX4_I2C(uint8_t bus) :
    I2C("AP_I2C", "/dev/api2c", bus, 0, 400000UL)
{
    init();
}

bool PX4_I2C::do_transfer(uint8_t address, const uint8_t *send, uint32_t send_len,
                          uint8_t *recv, uint32_t recv_len)
{
    set_address(address);
    return transfer(send, send_len, recv, recv_len) == OK;
}

bool I2CDevice::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    return _device.do_transfer(_address, send, send_len, recv, recv_len);
}

bool I2CDevice::read_registers_multiple(uint8_t first_reg, uint8_t *recv,
                                        uint32_t recv_len, uint8_t times)
{
    return true;
}

int I2CDevice::get_fd()
{
    return -1;
}

AP_HAL::Semaphore *I2CDevice::get_semaphore()
{
    return &semaphore;
}

I2CDevice::~I2CDevice()
{
}

I2CDeviceManager::I2CDeviceManager()
{
}

AP_HAL::OwnPtr<AP_HAL::I2CDevice>
I2CDeviceManager::get_device(uint8_t bus, uint8_t address)
{
    AP_HAL::OwnPtr<PX4_I2C> i2c { new PX4_I2C(bus) };
    auto dev = AP_HAL::OwnPtr<AP_HAL::I2CDevice>(new I2CDevice(*i2c, address));

    return dev;
}
}
