#pragma once

#define vkCmdBuildAccelerationStructuresKHR reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(*device->getDevice(), "vkCmdBuildAccelerationStructuresKHR"))
#define vkCreateAccelerationStructureKHR reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(*device->getDevice(), "vkCreateAccelerationStructureKHR"))
#define vkGetAccelerationStructureBuildSizesKHR reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(*device->getDevice(), "vkGetAccelerationStructureBuildSizesKHR"))