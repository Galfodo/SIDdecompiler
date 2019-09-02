
#include "MemoryMappedDevice.h"

namespace SASM {

MemoryMappedDevice::MemoryMappedDevice() : m_MachineState(nullptr), m_IsDirty(true) {
}

MemoryMappedDevice::~MemoryMappedDevice() {
}

void MemoryMappedDevice::onAttach() {}
void MemoryMappedDevice::onReadAccess(int address) {}
void MemoryMappedDevice::onWriteAccess(int address) {}
void MemoryMappedDevice::reset() {}
void MemoryMappedDevice::update(int delta_cycles) {}

}
