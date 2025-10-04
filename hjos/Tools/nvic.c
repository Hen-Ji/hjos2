#include "nvic.h"


void nvicSetGroup(uint8_t group) {
    uint32_t nvicGroups[] = {NVIC_PriorityGroup_0, NVIC_PriorityGroup_1, NVIC_PriorityGroup_2, NVIC_PriorityGroup_3, NVIC_PriorityGroup_4};
    NVIC_PriorityGroupConfig(nvicGroups[group]); //优先级分组
}