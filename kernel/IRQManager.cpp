#include "IRQManager.hpp"

static constexpr int MAX_IRQ_NUM = 32;
static IRQHandler handlers[MAX_IRQ_NUM];
int IRQManager::SetHandler(unsigned int irq, IRQHandler handler) {
	handlers[irq] = handler;
	return 0;
}

void IRQManager::Initialize() {
	for (int i = 0; i < MAX_IRQ_NUM; ++i) {
		handlers[i] = nullptr;
	}
}

void IRQManager::DoIRQ(unsigned int irq) {
	if (irq < MAX_IRQ_NUM) {
		IRQHandler handler = handlers[irq];
		if (handler != nullptr) {
			handler();
		}
	}
}
